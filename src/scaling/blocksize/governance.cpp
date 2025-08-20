#include <scaling/blocksize/governance.h>

#include <chain.h>
#include <consensus/params.h>
#include <primitives/block.h>
#include <logging.h>
#include <util/time.h>
#include <serialize.h>
#include <util/strencodings.h>
#include <consensus/validation.h>

#include <algorithm>
#include <numeric>
#include <cmath>

namespace blocksize {

// Global governance state
static BlockSizeState g_governance_state;

uint64_t GetBlockSizeLimit(int height, const Consensus::Params& params) {
    // Before governance activation, use base block size
    if (!IsGovernanceActive(height, params)) {
        return governance::BASE_BLOCK_SIZE;
    }
    
    // Return current governance limit
    return g_governance_state.current_limit;
}

uint64_t CalculateTargetBlockSize(const CBlockIndex* pindex, const Consensus::Params& params) {
    if (!pindex || !IsGovernanceActive(pindex->nHeight, params)) {
        return governance::BASE_BLOCK_SIZE;
    }
    
    // Get recent block size statistics
    BlockSizeStats stats = GetBlockSizeStats(pindex, governance::MIN_SAMPLE_SIZE, params);
    
    // Calculate target based on utilization
    uint64_t target = stats.average_size;
    
    // If utilization is high, increase target
    if (stats.utilization_rate > 0.8) {
        target = static_cast<uint64_t>(target * 1.5);
    }
    // If utilization is low, decrease target
    else if (stats.utilization_rate < 0.3) {
        target = static_cast<uint64_t>(target * 0.8);
    }
    
    // Ensure target is within reasonable bounds
    target = std::max(target, governance::BASE_BLOCK_SIZE);
    target = std::min(target, g_governance_state.current_limit * 2);
    
    return target;
}

void UpdateGovernanceState(const CBlock& block, const CBlockIndex* pindex, 
                          BlockSizeState& state, const Consensus::Params& params) {
    if (!IsGovernanceActive(pindex->nHeight, params)) {
        return;
    }
    
    // Get block size
    uint64_t block_size = GetBlockWeight(block);
    
    // Add to recent sizes (keep last 100 blocks)
    state.recent_sizes.push_back(block_size);
    if (state.recent_sizes.size() > 100) {
        state.recent_sizes.erase(state.recent_sizes.begin());
    }
    
    state.blocks_since_adjustment++;
    
    // Check if it's time for adjustment
    if (state.blocks_since_adjustment >= governance::ADJUSTMENT_PERIOD) {
        // Calculate new target
        state.target_size = CalculateTargetBlockSize(pindex, params);
        
        // Calculate adjustment factor
        state.adjustment_factor = CalculateAdjustmentFactor(state, pindex, params);
        
        // Apply adjustment
        uint64_t new_limit = static_cast<uint64_t>(state.current_limit * state.adjustment_factor);
        
        // Ensure bounds
        new_limit = std::max(new_limit, governance::BASE_BLOCK_SIZE);
        
        LogPrintf("BlockSize Governance: Adjusting limit from %lu to %lu (factor: %.2f)\n",
                 state.current_limit, new_limit, state.adjustment_factor);
        
        state.current_limit = new_limit;
        state.blocks_since_adjustment = 0;
    }
    
    // Check emergency scaling
    if (ShouldActivateEmergencyScaling(state, pindex, params)) {
        if (!state.emergency_mode) {
            LogPrintf("BlockSize Governance: Activating emergency scaling\n");
            state.emergency_mode = true;
            state.current_limit = static_cast<uint64_t>(state.current_limit * governance::EMERGENCY_MULTIPLIER);
        }
    } else {
        if (state.emergency_mode) {
            LogPrintf("BlockSize Governance: Deactivating emergency scaling\n");
            state.emergency_mode = false;
        }
    }
    
    LogGovernanceState(state, pindex->nHeight);
}

bool ShouldActivateEmergencyScaling(const BlockSizeState& state, 
                                   const CBlockIndex* pindex, const Consensus::Params& params) {
    if (state.recent_sizes.size() < governance::MIN_SAMPLE_SIZE) {
        return false;
    }
    
    // Count blocks at or near capacity
    int blocks_at_capacity = 0;
    uint64_t capacity_threshold = static_cast<uint64_t>(state.current_limit * 0.95);
    
    for (uint64_t size : state.recent_sizes) {
        if (size >= capacity_threshold) {
            blocks_at_capacity++;
        }
    }
    
    double capacity_rate = static_cast<double>(blocks_at_capacity) / state.recent_sizes.size();
    return capacity_rate >= governance::EMERGENCY_THRESHOLD;
}

double CalculateAdjustmentFactor(const BlockSizeState& state, 
                                const CBlockIndex* pindex, const Consensus::Params& params) {
    if (state.recent_sizes.empty()) {
        return 1.0;
    }
    
    // Calculate average utilization
    uint64_t total_size = std::accumulate(state.recent_sizes.begin(), state.recent_sizes.end(), 0ULL);
    double avg_utilization = static_cast<double>(total_size) / 
                            (state.recent_sizes.size() * state.current_limit);
    
    double factor = 1.0;
    
    // Increase if high utilization
    if (avg_utilization > 0.8) {
        factor = 1.0 + (avg_utilization - 0.8) * 2.0; // Scale up to 1.4x
    }
    // Decrease if low utilization
    else if (avg_utilization < 0.3) {
        factor = 0.5 + avg_utilization * 1.67; // Scale down to 0.5x
    }
    
    // Apply bounds
    factor = std::min(factor, governance::MAX_INCREASE_FACTOR);
    factor = std::max(factor, governance::MAX_DECREASE_FACTOR);
    
    return factor;
}

bool ValidateBlockSizeGovernance(const CBlock& block, const CBlockIndex* pindex, 
                                const Consensus::Params& params, std::string& error) {
    if (!IsGovernanceActive(pindex->nHeight, params)) {
        // Before governance, use standard validation
        return true;
    }
    
    uint64_t block_size = GetBlockWeight(block);
    uint64_t size_limit = GetBlockSizeLimit(pindex->nHeight, params);
    
    if (block_size > size_limit) {
        error = strprintf("Block size %lu exceeds governance limit %lu", block_size, size_limit);
        return false;
    }
    
    return true;
}

bool ExtractMinerVote(const CBlock& block, MinerVote& vote) {
    // Look for miner vote in coinbase transaction
    if (block.vtx.empty()) {
        return false;
    }
    
    const CTransaction& coinbase = *block.vtx[0];
    if (coinbase.vin.empty()) {
        return false;
    }
    
    // Parse coinbase script for vote data
    // Format: OP_RETURN <version> <preferred_size> <max_size>
    const CScript& script = coinbase.vin[0].scriptSig;
    
    // Simple implementation - in practice would need more robust parsing
    if (script.size() >= 16) {
        // Extract vote data (simplified)
        vote.preferred_size = governance::BASE_BLOCK_SIZE * 2; // Placeholder
        vote.max_size = governance::BASE_BLOCK_SIZE * 4;       // Placeholder
        vote.timestamp = GetTime();
        return true;
    }
    
    return false;
}

uint64_t CalculateConsensusBlockSize(const std::vector<MinerVote>& votes, 
                                    const CBlockIndex* pindex, const Consensus::Params& params) {
    if (votes.empty()) {
        return governance::BASE_BLOCK_SIZE;
    }
    
    // Calculate median of preferred sizes
    std::vector<uint64_t> sizes;
    for (const auto& vote : votes) {
        sizes.push_back(vote.preferred_size);
    }
    
    std::sort(sizes.begin(), sizes.end());
    size_t mid = sizes.size() / 2;
    
    if (sizes.size() % 2 == 0) {
        return (sizes[mid - 1] + sizes[mid]) / 2;
    } else {
        return sizes[mid];
    }
}

BlockSizeStats GetBlockSizeStats(const CBlockIndex* pindex, int sample_size, 
                                const Consensus::Params& params) {
    BlockSizeStats stats = {};
    
    if (!pindex) {
        return stats;
    }
    
    std::vector<uint64_t> sizes;
    const CBlockIndex* current = pindex;
    
    // Collect block sizes
    for (int i = 0; i < sample_size && current && current->pprev; ++i) {
        // In practice, would read actual block size from disk
        // For now, use a placeholder calculation
        uint64_t estimated_size = governance::BASE_BLOCK_SIZE / 2 + 
                                 (current->nHeight % 1000) * 500; // Placeholder
        sizes.push_back(estimated_size);
        current = current->pprev;
    }
    
    if (sizes.empty()) {
        return stats;
    }
    
    // Calculate statistics
    stats.average_size = std::accumulate(sizes.begin(), sizes.end(), 0ULL) / sizes.size();
    
    std::sort(sizes.begin(), sizes.end());
    stats.median_size = sizes[sizes.size() / 2];
    stats.max_size = sizes.back();
    stats.min_size = sizes.front();
    
    // Calculate utilization rate
    uint64_t current_limit = GetBlockSizeLimit(pindex->nHeight, params);
    stats.utilization_rate = static_cast<double>(stats.average_size) / current_limit;
    
    // Count blocks at capacity
    uint64_t capacity_threshold = static_cast<uint64_t>(current_limit * 0.95);
    stats.blocks_at_capacity = std::count_if(sizes.begin(), sizes.end(),
        [capacity_threshold](uint64_t size) { return size >= capacity_threshold; });
    
    return stats;
}

void InitializeBlockSizeGovernance(const Consensus::Params& params) {
    LogPrintf("BlockSize Governance: Initializing unbounded block size governance\n");
    
    // Initialize global state
    g_governance_state = BlockSizeState();
    
    LogPrintf("BlockSize Governance: Base block size: %lu bytes\n", governance::BASE_BLOCK_SIZE);
    LogPrintf("BlockSize Governance: Adjustment period: %d blocks\n", governance::ADJUSTMENT_PERIOD);
    LogPrintf("BlockSize Governance: Activation height: %d\n", governance::GOVERNANCE_ACTIVATION_HEIGHT);
}

bool IsGovernanceActive(int height, const Consensus::Params& params) {
    return height >= GetGovernanceActivationHeight(params);
}

int GetGovernanceActivationHeight(const Consensus::Params& params) {
    return governance::GOVERNANCE_ACTIVATION_HEIGHT;
}

void LogGovernanceState(const BlockSizeState& state, int height) {
    LogPrintf("BlockSize Governance [%d]: limit=%lu, target=%lu, factor=%.2f, emergency=%s, blocks=%d\n",
             height, state.current_limit, state.target_size, state.adjustment_factor,
             state.emergency_mode ? "yes" : "no", state.blocks_since_adjustment);
}

} // namespace blocksize
