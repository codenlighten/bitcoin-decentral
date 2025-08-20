#ifndef BITCOIN_SCALING_BLOCKSIZE_GOVERNANCE_H
#define BITCOIN_SCALING_BLOCKSIZE_GOVERNANCE_H

#include <cstdint>
#include <vector>
#include <string>

class CBlockIndex;
class CBlock;
namespace Consensus { struct Params; }

/**
 * Bitcoin Decentral Unbounded Block Size Governance
 * 
 * This module implements dynamic block size governance that removes Bitcoin's
 * artificial 1MB block size limit and allows the network to scale based on
 * actual demand and network capacity.
 * 
 * Key Features:
 * - Dynamic block size adjustment based on network conditions
 * - Miner voting mechanism for block size preferences
 * - Gradual adjustment algorithm to prevent sudden changes
 * - Emergency governance for rapid scaling when needed
 * - Backward compatibility with existing block size validation
 */

namespace blocksize {

/**
 * Block size governance constants
 */
namespace governance {
    // Base block size (1MB in bytes)
    static const uint64_t BASE_BLOCK_SIZE = 1000000;
    
    // Maximum block size increase per adjustment period (100% = double)
    static const double MAX_INCREASE_FACTOR = 2.0;
    
    // Maximum block size decrease per adjustment period (50% = half)
    static const double MAX_DECREASE_FACTOR = 0.5;
    
    // Block size adjustment period (blocks)
    static const int ADJUSTMENT_PERIOD = 2016; // ~2 weeks
    
    // Minimum number of blocks for size calculation
    static const int MIN_SAMPLE_SIZE = 100;
    
    // Emergency scaling threshold (percentage of blocks at capacity)
    static const double EMERGENCY_THRESHOLD = 0.95; // 95%
    
    // Emergency scaling multiplier
    static const double EMERGENCY_MULTIPLIER = 4.0;
    
    // Governance activation height
    static const int GOVERNANCE_ACTIVATION_HEIGHT = 2000;
}

/**
 * Block size governance state
 */
struct BlockSizeState {
    uint64_t current_limit;        // Current block size limit
    uint64_t target_size;          // Target size based on recent usage
    double adjustment_factor;      // Current adjustment factor
    bool emergency_mode;           // Emergency scaling active
    int blocks_since_adjustment;   // Blocks since last adjustment
    std::vector<uint64_t> recent_sizes; // Recent block sizes for analysis
    
    BlockSizeState() : 
        current_limit(governance::BASE_BLOCK_SIZE),
        target_size(governance::BASE_BLOCK_SIZE),
        adjustment_factor(1.0),
        emergency_mode(false),
        blocks_since_adjustment(0) {}
};

/**
 * Miner voting for block size preferences
 */
struct MinerVote {
    uint64_t preferred_size;       // Miner's preferred block size
    uint64_t max_size;            // Miner's maximum acceptable size
    uint32_t timestamp;           // Vote timestamp
    
    MinerVote(uint64_t pref, uint64_t max, uint32_t time) :
        preferred_size(pref), max_size(max), timestamp(time) {}
};

/**
 * Core governance functions
 */

/**
 * Calculate the current block size limit for a given height
 */
uint64_t GetBlockSizeLimit(int height, const Consensus::Params& params);

/**
 * Calculate the target block size based on recent network usage
 */
uint64_t CalculateTargetBlockSize(const CBlockIndex* pindex, const Consensus::Params& params);

/**
 * Update block size governance state with new block
 */
void UpdateGovernanceState(const CBlock& block, const CBlockIndex* pindex, 
                          BlockSizeState& state, const Consensus::Params& params);

/**
 * Check if emergency scaling should be activated
 */
bool ShouldActivateEmergencyScaling(const BlockSizeState& state, 
                                   const CBlockIndex* pindex, const Consensus::Params& params);

/**
 * Calculate block size adjustment factor
 */
double CalculateAdjustmentFactor(const BlockSizeState& state, 
                                const CBlockIndex* pindex, const Consensus::Params& params);

/**
 * Validate block size against governance rules
 */
bool ValidateBlockSizeGovernance(const CBlock& block, const CBlockIndex* pindex, 
                                const Consensus::Params& params, std::string& error);

/**
 * Extract miner vote from block (if present)
 */
bool ExtractMinerVote(const CBlock& block, MinerVote& vote);

/**
 * Calculate consensus block size from miner votes
 */
uint64_t CalculateConsensusBlockSize(const std::vector<MinerVote>& votes, 
                                    const CBlockIndex* pindex, const Consensus::Params& params);

/**
 * Get block size statistics for analysis
 */
struct BlockSizeStats {
    uint64_t average_size;
    uint64_t median_size;
    uint64_t max_size;
    uint64_t min_size;
    double utilization_rate;
    int blocks_at_capacity;
};

BlockSizeStats GetBlockSizeStats(const CBlockIndex* pindex, int sample_size, 
                                const Consensus::Params& params);

/**
 * Initialize block size governance
 */
void InitializeBlockSizeGovernance(const Consensus::Params& params);

/**
 * Check if block size governance is active at given height
 */
bool IsGovernanceActive(int height, const Consensus::Params& params);

/**
 * Get governance activation height
 */
int GetGovernanceActivationHeight(const Consensus::Params& params);

/**
 * Log governance state for debugging
 */
void LogGovernanceState(const BlockSizeState& state, int height);

} // namespace blocksize

#endif // BITCOIN_SCALING_BLOCKSIZE_GOVERNANCE_H
