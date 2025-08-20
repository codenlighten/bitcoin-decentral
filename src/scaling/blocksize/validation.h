#ifndef BITCOIN_SCALING_BLOCKSIZE_VALIDATION_H
#define BITCOIN_SCALING_BLOCKSIZE_VALIDATION_H

#include <cstdint>
#include <string>

class CBlock;
class CBlockIndex;
class CTxMemPool;
namespace Consensus { struct Params; }

/**
 * Bitcoin Decentral Unbounded Block Size Validation
 * 
 * This module provides validation functions for the unbounded block size
 * governance system, ensuring blocks comply with dynamic size limits
 * while maintaining network security and stability.
 */

namespace blocksize {

/**
 * Validate block size against governance rules
 * 
 * @param block The block to validate
 * @param pindex Block index for context
 * @param params Consensus parameters
 * @param error Error message if validation fails
 * @return true if block size is valid, false otherwise
 */
bool ValidateBlockSize(const CBlock& block, const CBlockIndex* pindex, 
                      const Consensus::Params& params, std::string& error);

/**
 * Validate block size for mempool acceptance
 * 
 * @param block The block to validate
 * @param mempool Current mempool state
 * @param params Consensus parameters
 * @param error Error message if validation fails
 * @return true if block would be acceptable, false otherwise
 */
bool ValidateBlockSizeForMempool(const CBlock& block, const CTxMemPool& mempool,
                                const Consensus::Params& params, std::string& error);

/**
 * Check if a block size increase is justified
 * 
 * @param current_limit Current block size limit
 * @param proposed_size Proposed new block size
 * @param pindex Block index for context
 * @param params Consensus parameters
 * @return true if increase is justified, false otherwise
 */
bool IsBlockSizeIncreaseJustified(uint64_t current_limit, uint64_t proposed_size,
                                 const CBlockIndex* pindex, const Consensus::Params& params);

/**
 * Validate governance transition
 * 
 * @param old_limit Previous block size limit
 * @param new_limit New block size limit
 * @param height Current block height
 * @param params Consensus parameters
 * @param error Error message if validation fails
 * @return true if transition is valid, false otherwise
 */
bool ValidateGovernanceTransition(uint64_t old_limit, uint64_t new_limit, int height,
                                 const Consensus::Params& params, std::string& error);

/**
 * Check block size against network capacity
 * 
 * @param block_size Size of the block
 * @param pindex Block index for context
 * @param params Consensus parameters
 * @return true if network can handle the block size, false otherwise
 */
bool CheckNetworkCapacity(uint64_t block_size, const CBlockIndex* pindex,
                         const Consensus::Params& params);

/**
 * Validate miner vote for block size
 * 
 * @param vote Miner vote to validate
 * @param pindex Block index for context
 * @param params Consensus parameters
 * @param error Error message if validation fails
 * @return true if vote is valid, false otherwise
 */
bool ValidateMinerVote(const struct MinerVote& vote, const CBlockIndex* pindex,
                      const Consensus::Params& params, std::string& error);

/**
 * Check if emergency scaling is warranted
 * 
 * @param pindex Block index for context
 * @param params Consensus parameters
 * @return true if emergency scaling should be activated, false otherwise
 */
bool ShouldActivateEmergencyScaling(const CBlockIndex* pindex, const Consensus::Params& params);

/**
 * Validate block size adjustment
 * 
 * @param adjustment_factor Proposed adjustment factor
 * @param current_state Current governance state
 * @param pindex Block index for context
 * @param params Consensus parameters
 * @param error Error message if validation fails
 * @return true if adjustment is valid, false otherwise
 */
bool ValidateBlockSizeAdjustment(double adjustment_factor, const struct BlockSizeState& current_state,
                                const CBlockIndex* pindex, const Consensus::Params& params,
                                std::string& error);

/**
 * Check consensus requirements for block size changes
 * 
 * @param proposed_limit Proposed new block size limit
 * @param pindex Block index for context
 * @param params Consensus parameters
 * @return true if consensus requirements are met, false otherwise
 */
bool CheckConsensusRequirements(uint64_t proposed_limit, const CBlockIndex* pindex,
                               const Consensus::Params& params);

/**
 * Validate block size governance activation
 * 
 * @param height Block height
 * @param params Consensus parameters
 * @param error Error message if validation fails
 * @return true if governance can be activated, false otherwise
 */
bool ValidateGovernanceActivation(int height, const Consensus::Params& params, std::string& error);

} // namespace blocksize

#endif // BITCOIN_SCALING_BLOCKSIZE_VALIDATION_H
