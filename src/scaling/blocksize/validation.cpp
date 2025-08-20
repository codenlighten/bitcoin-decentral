#include <scaling/blocksize/validation.h>
#include <scaling/blocksize/governance.h>

#include <chain.h>
#include <consensus/params.h>
#include <primitives/block.h>
#include <txmempool.h>
#include <logging.h>
#include <util/strencodings.h>
#include <serialize.h>
#include <consensus/validation.h>

#include <algorithm>
#include <cmath>

namespace blocksize {

bool ValidateBlockSize(const CBlock& block, const CBlockIndex* pindex, 
                      const Consensus::Params& params, std::string& error) {
    if (!pindex) {
        error = "Invalid block index for block size validation";
        return false;
    }
    
    // Get block size
    uint64_t block_size = GetBlockWeight(block);
    
    // Get current governance limit
    uint64_t size_limit = GetBlockSizeLimit(pindex->nHeight, params);
    
    // Validate against governance rules
    if (!ValidateBlockSizeGovernance(block, pindex, params, error)) {
        return false;
    }
    
    // Check network capacity
    if (!CheckNetworkCapacity(block_size, pindex, params)) {
        error = strprintf("Block size %lu exceeds network capacity", block_size);
        return false;
    }
    
    // Additional validation for governance transitions
    if (pindex->pprev) {
        uint64_t prev_limit = GetBlockSizeLimit(pindex->pprev->nHeight, params);
        if (!ValidateGovernanceTransition(prev_limit, size_limit, pindex->nHeight, params, error)) {
            return false;
        }
    }
    
    LogPrintf("BlockSize Validation: Block %s size %lu validated against limit %lu\n",
             block.GetHash().ToString(), block_size, size_limit);
    
    return true;
}

bool ValidateBlockSizeForMempool(const CBlock& block, const CTxMemPool& mempool,
                                const Consensus::Params& params, std::string& error) {
    // Get estimated block size
    uint64_t block_size = GetBlockWeight(block);
    
    // Get current mempool size
    size_t mempool_size = mempool.GetTotalTxSize();
    
    // Estimate if block would be acceptable
    uint64_t estimated_limit = governance::BASE_BLOCK_SIZE * 2; // Conservative estimate
    
    if (block_size > estimated_limit) {
        error = strprintf("Block size %lu would exceed estimated limit %lu", 
                         block_size, estimated_limit);
        return false;
    }
    
    // Check if block size is reasonable given mempool state
    if (mempool_size > 0 && block_size < mempool_size / 10) {
        // Block is suspiciously small given mempool size
        LogPrintf("BlockSize Validation: Warning - block size %lu is small for mempool size %lu\n",
                 block_size, mempool_size);
    }
    
    return true;
}

bool IsBlockSizeIncreaseJustified(uint64_t current_limit, uint64_t proposed_size,
                                 const CBlockIndex* pindex, const Consensus::Params& params) {
    if (!pindex || proposed_size <= current_limit) {
        return false;
    }
    
    // Get recent block size statistics
    BlockSizeStats stats = GetBlockSizeStats(pindex, 100, params);
    
    // Increase is justified if:
    // 1. Recent utilization is high (>80%)
    // 2. Many blocks are at capacity (>50%)
    // 3. Proposed increase is reasonable (<2x current limit)
    
    bool high_utilization = stats.utilization_rate > 0.8;
    bool many_at_capacity = stats.blocks_at_capacity > 50;
    bool reasonable_increase = proposed_size <= current_limit * 2;
    
    return high_utilization && many_at_capacity && reasonable_increase;
}

bool ValidateGovernanceTransition(uint64_t old_limit, uint64_t new_limit, int height,
                                 const Consensus::Params& params, std::string& error) {
    // Check if governance is active
    if (!IsGovernanceActive(height, params)) {
        if (new_limit != governance::BASE_BLOCK_SIZE) {
            error = strprintf("Governance not active at height %d, limit must be %lu", 
                             height, governance::BASE_BLOCK_SIZE);
            return false;
        }
        return true;
    }
    
    // Check transition bounds
    double change_factor = static_cast<double>(new_limit) / old_limit;
    
    if (change_factor > governance::MAX_INCREASE_FACTOR) {
        error = strprintf("Block size increase factor %.2f exceeds maximum %.2f", 
                         change_factor, governance::MAX_INCREASE_FACTOR);
        return false;
    }
    
    if (change_factor < governance::MAX_DECREASE_FACTOR) {
        error = strprintf("Block size decrease factor %.2f below minimum %.2f", 
                         change_factor, governance::MAX_DECREASE_FACTOR);
        return false;
    }
    
    // Check minimum size
    if (new_limit < governance::BASE_BLOCK_SIZE) {
        error = strprintf("Block size limit %lu below minimum %lu", 
                         new_limit, governance::BASE_BLOCK_SIZE);
        return false;
    }
    
    return true;
}

bool CheckNetworkCapacity(uint64_t block_size, const CBlockIndex* pindex,
                         const Consensus::Params& params) {
    // Simple network capacity check
    // In practice, this would consider:
    // - Network bandwidth
    // - Node processing capacity
    // - Storage requirements
    // - Propagation time
    
    // For now, use a conservative maximum
    const uint64_t MAX_NETWORK_BLOCK_SIZE = 100 * 1024 * 1024; // 100MB
    
    if (block_size > MAX_NETWORK_BLOCK_SIZE) {
        LogPrintf("BlockSize Validation: Block size %lu exceeds network capacity limit %lu\n",
                 block_size, MAX_NETWORK_BLOCK_SIZE);
        return false;
    }
    
    return true;
}

bool ValidateMinerVote(const MinerVote& vote, const CBlockIndex* pindex,
                      const Consensus::Params& params, std::string& error) {
    // Validate vote parameters
    if (vote.preferred_size < governance::BASE_BLOCK_SIZE) {
        error = strprintf("Miner preferred size %lu below minimum %lu", 
                         vote.preferred_size, governance::BASE_BLOCK_SIZE);
        return false;
    }
    
    if (vote.max_size < vote.preferred_size) {
        error = strprintf("Miner max size %lu less than preferred size %lu", 
                         vote.max_size, vote.preferred_size);
        return false;
    }
    
    // Check reasonableness
    const uint64_t MAX_REASONABLE_SIZE = 1024 * 1024 * 1024; // 1GB
    if (vote.max_size > MAX_REASONABLE_SIZE) {
        error = strprintf("Miner max size %lu exceeds reasonable limit %lu", 
                         vote.max_size, MAX_REASONABLE_SIZE);
        return false;
    }
    
    return true;
}

bool ShouldActivateEmergencyScaling(const CBlockIndex* pindex, const Consensus::Params& params) {
    if (!pindex || !IsGovernanceActive(pindex->nHeight, params)) {
        return false;
    }
    
    // Get recent block statistics
    BlockSizeStats stats = GetBlockSizeStats(pindex, governance::MIN_SAMPLE_SIZE, params);
    
    // Emergency scaling if >95% of recent blocks are at capacity
    double capacity_rate = static_cast<double>(stats.blocks_at_capacity) / governance::MIN_SAMPLE_SIZE;
    
    return capacity_rate >= governance::EMERGENCY_THRESHOLD;
}

bool ValidateBlockSizeAdjustment(double adjustment_factor, const BlockSizeState& current_state,
                                const CBlockIndex* pindex, const Consensus::Params& params,
                                std::string& error) {
    // Check adjustment bounds
    if (adjustment_factor > governance::MAX_INCREASE_FACTOR) {
        error = strprintf("Adjustment factor %.2f exceeds maximum %.2f", 
                         adjustment_factor, governance::MAX_INCREASE_FACTOR);
        return false;
    }
    
    if (adjustment_factor < governance::MAX_DECREASE_FACTOR) {
        error = strprintf("Adjustment factor %.2f below minimum %.2f", 
                         adjustment_factor, governance::MAX_DECREASE_FACTOR);
        return false;
    }
    
    // Check if adjustment is premature
    if (current_state.blocks_since_adjustment < governance::ADJUSTMENT_PERIOD) {
        error = strprintf("Adjustment attempted after %d blocks, minimum is %d", 
                         current_state.blocks_since_adjustment, governance::ADJUSTMENT_PERIOD);
        return false;
    }
    
    // Validate resulting size
    uint64_t new_limit = static_cast<uint64_t>(current_state.current_limit * adjustment_factor);
    if (new_limit < governance::BASE_BLOCK_SIZE) {
        error = strprintf("Adjusted limit %lu below minimum %lu", 
                         new_limit, governance::BASE_BLOCK_SIZE);
        return false;
    }
    
    return true;
}

bool CheckConsensusRequirements(uint64_t proposed_limit, const CBlockIndex* pindex,
                               const Consensus::Params& params) {
    // In a full implementation, this would check:
    // - Miner signaling/voting
    // - Economic node consensus
    // - Network upgrade readiness
    
    // For now, accept reasonable proposals
    uint64_t current_limit = GetBlockSizeLimit(pindex->nHeight, params);
    double change_factor = static_cast<double>(proposed_limit) / current_limit;
    
    // Require consensus for large changes
    if (change_factor > 1.5 || change_factor < 0.7) {
        LogPrintf("BlockSize Validation: Large change factor %.2f requires consensus\n", change_factor);
        return false; // Would require additional consensus mechanism
    }
    
    return true;
}

bool ValidateGovernanceActivation(int height, const Consensus::Params& params, std::string& error) {
    int activation_height = GetGovernanceActivationHeight(params);
    
    if (height < activation_height) {
        error = strprintf("Governance activation attempted at height %d, required height is %d", 
                         height, activation_height);
        return false;
    }
    
    // Additional checks could include:
    // - Network readiness
    // - Miner signaling
    // - Economic consensus
    
    return true;
}

} // namespace blocksize
