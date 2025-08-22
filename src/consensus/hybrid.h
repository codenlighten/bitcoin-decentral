#ifndef BITCOIN_CONSENSUS_HYBRID_H
#define BITCOIN_CONSENSUS_HYBRID_H

#include <cstdint>
#include <vector>
#include <map>
#include <memory>

class CBlock;
class CBlockIndex;
class CTransaction;
class uint256;
namespace Consensus { struct Params; }

/**
 * Bitcoin Decentral Hybrid Consensus System
 * 
 * This module implements a revolutionary hybrid Proof-of-Work/Proof-of-Stake
 * consensus mechanism that combines the security of PoW with the efficiency
 * and finality of PoS.
 * 
 * Key Features:
 * - Dual consensus validation with PoW mining and PoS checkpoints
 * - Fast finality through validator network consensus
 * - Enhanced security through economic staking incentives
 * - Gradual transition from pure PoW to hybrid consensus
 * - Slashing conditions for malicious validator behavior
 * - On-chain governance through validator voting
 */

namespace consensus {
namespace hybrid {

/**
 * Hybrid consensus constants
 */
namespace params {
    // Minimum stake required to become a validator (in satoshis)
    static const uint64_t MIN_VALIDATOR_STAKE = 100000 * 100000000ULL; // 100,000 BTCD
    
    // Maximum number of active validators
    static const int MAX_ACTIVE_VALIDATORS = 100;
    
    // Minimum number of validators for consensus
    static const int MIN_VALIDATORS_FOR_CONSENSUS = 67; // 2/3 + 1
    
    // Checkpoint interval (blocks)
    static const int CHECKPOINT_INTERVAL = 100;
    
    // Validator reward per checkpoint (in satoshis)
    static const uint64_t VALIDATOR_REWARD = 1000000; // 0.01 BTCD
    
    // Slashing penalty percentage
    static const double SLASHING_PENALTY = 0.1; // 10%
    
    // Hybrid consensus activation height
    static const int HYBRID_ACTIVATION_HEIGHT = 10000;
    
    // Finality confirmation blocks
    static const int FINALITY_CONFIRMATION_BLOCKS = 6;
}

/**
 * Validator information
 */
struct ValidatorInfo {
    uint256 validator_id;           // Unique validator identifier
    uint256 public_key;             // Validator public key
    uint64_t stake_amount;          // Amount staked (in satoshis)
    uint32_t activation_height;     // Height when validator became active
    uint32_t last_checkpoint;       // Last checkpoint participated in
    bool is_active;                 // Whether validator is currently active
    double reputation_score;        // Validator reputation (0.0 - 1.0)
    uint32_t missed_checkpoints;    // Number of missed checkpoints
    uint64_t total_rewards;         // Total rewards earned
    uint64_t slashed_amount;        // Total amount slashed
    
    ValidatorInfo() : stake_amount(0), activation_height(0), last_checkpoint(0),
                     is_active(false), reputation_score(1.0), missed_checkpoints(0),
                     total_rewards(0), slashed_amount(0) {}
};

/**
 * Checkpoint information
 */
struct CheckpointInfo {
    uint32_t height;                // Checkpoint block height
    uint256 block_hash;             // Block hash at checkpoint
    uint256 checkpoint_hash;        // Checkpoint hash
    std::vector<uint256> validator_signatures; // Validator signatures
    uint32_t timestamp;             // Checkpoint timestamp
    bool is_finalized;              // Whether checkpoint is finalized
    uint32_t confirmation_count;    // Number of confirmations
    
    CheckpointInfo() : height(0), timestamp(0), is_finalized(false), confirmation_count(0) {}
};

/**
 * Staking transaction
 */
struct StakingTransaction {
    uint256 txid;                   // Transaction ID
    uint256 validator_id;           // Validator ID
    uint64_t stake_amount;          // Stake amount
    uint32_t lock_time;             // Stake lock time
    bool is_withdrawal;             // Whether this is a withdrawal
    uint32_t creation_height;       // Height when stake was created
    
    StakingTransaction() : stake_amount(0), lock_time(0), is_withdrawal(false), creation_height(0) {}
};

/**
 * Validator network state
 */
struct ValidatorNetworkState {
    std::map<uint256, ValidatorInfo> active_validators;     // Active validators
    std::map<uint256, ValidatorInfo> pending_validators;    // Pending validators
    std::vector<CheckpointInfo> recent_checkpoints;         // Recent checkpoints
    uint64_t total_staked_amount;                          // Total staked amount
    uint32_t current_epoch;                                // Current consensus epoch
    uint32_t last_checkpoint_height;                       // Last checkpoint height
    
    ValidatorNetworkState() : total_staked_amount(0), current_epoch(0), last_checkpoint_height(0) {}
};

/**
 * Core hybrid consensus functions
 */

/**
 * Initialize hybrid consensus system
 */
bool InitializeHybridConsensus(const Consensus::Params& params);

/**
 * Check if hybrid consensus is active at given height
 */
bool IsHybridConsensusActive(int height, const Consensus::Params& params);

/**
 * Get hybrid consensus activation height
 */
int GetHybridActivationHeight(const Consensus::Params& params);

/**
 * Validate block with hybrid consensus
 */
bool ValidateBlockHybridConsensus(const CBlock& block, const CBlockIndex* pindex,
                                 const Consensus::Params& params);

/**
 * Create checkpoint for given block
 */
bool CreateCheckpoint(const CBlock& block, const CBlockIndex* pindex,
                     CheckpointInfo& checkpoint);

/**
 * Validate checkpoint signatures
 */
bool ValidateCheckpointSignatures(const CheckpointInfo& checkpoint,
                                 const ValidatorNetworkState& validator_state);

/**
 * Finalize checkpoint
 */
bool FinalizeCheckpoint(CheckpointInfo& checkpoint,
                       ValidatorNetworkState& validator_state);

/**
 * Check if block is finalized
 */
bool IsBlockFinalized(const uint256& block_hash, const CBlockIndex* pindex);

/**
 * Validator management functions
 */

/**
 * Register new validator
 */
bool RegisterValidator(const uint256& validator_id, const uint256& public_key,
                      uint64_t stake_amount, ValidatorNetworkState& state);

/**
 * Activate pending validator
 */
bool ActivateValidator(const uint256& validator_id, ValidatorNetworkState& state);

/**
 * Deactivate validator
 */
bool DeactivateValidator(const uint256& validator_id, ValidatorNetworkState& state);

/**
 * Update validator stake
 */
bool UpdateValidatorStake(const uint256& validator_id, uint64_t new_stake_amount,
                         ValidatorNetworkState& state);

/**
 * Slash validator for malicious behavior
 */
bool SlashValidator(const uint256& validator_id, double penalty_percentage,
                   const std::string& reason, ValidatorNetworkState& state);

/**
 * Calculate validator rewards
 */
uint64_t CalculateValidatorRewards(const uint256& validator_id,
                                  const ValidatorNetworkState& state);

/**
 * Distribute validator rewards
 */
bool DistributeValidatorRewards(const CheckpointInfo& checkpoint,
                               ValidatorNetworkState& state);

/**
 * Staking functions
 */

/**
 * Process staking transaction
 */
bool ProcessStakingTransaction(const CTransaction& tx, ValidatorNetworkState& state);

/**
 * Validate staking transaction
 */
bool ValidateStakingTransaction(const CTransaction& tx, const Consensus::Params& params);

/**
 * Calculate minimum stake requirement
 */
uint64_t CalculateMinimumStake(int height, const Consensus::Params& params);

/**
 * Check if stake is locked
 */
bool IsStakeLocked(const StakingTransaction& stake, int current_height);

/**
 * Process stake withdrawal
 */
bool ProcessStakeWithdrawal(const uint256& validator_id, uint64_t amount,
                           ValidatorNetworkState& state);

/**
 * Consensus and finality functions
 */

/**
 * Check consensus threshold
 */
bool CheckConsensusThreshold(const std::vector<uint256>& validator_signatures,
                            const ValidatorNetworkState& state);

/**
 * Calculate finality score
 */
double CalculateFinalityScore(const uint256& block_hash, const CBlockIndex* pindex,
                             const ValidatorNetworkState& state);

/**
 * Get finality status
 */
enum FinalityStatus {
    FINALITY_PENDING,
    FINALITY_PROBABLE,
    FINALITY_CONFIRMED
};

FinalityStatus GetFinalityStatus(const uint256& block_hash, const CBlockIndex* pindex);

/**
 * Governance functions
 */

/**
 * Process governance proposal
 */
bool ProcessGovernanceProposal(const CTransaction& tx, ValidatorNetworkState& state);

/**
 * Vote on governance proposal
 */
bool VoteOnProposal(const uint256& proposal_id, const uint256& validator_id,
                   bool vote, ValidatorNetworkState& state);

/**
 * Execute approved proposal
 */
bool ExecuteApprovedProposal(const uint256& proposal_id, ValidatorNetworkState& state);

/**
 * Utility functions
 */

/**
 * Get validator network state
 */
ValidatorNetworkState GetValidatorNetworkState();

/**
 * Update validator network state
 */
bool UpdateValidatorNetworkState(const ValidatorNetworkState& state);

/**
 * Get active validator count
 */
int GetActiveValidatorCount(const ValidatorNetworkState& state);

/**
 * Get total staked amount
 */
uint64_t GetTotalStakedAmount(const ValidatorNetworkState& state);

/**
 * Calculate validator selection probability
 */
double CalculateValidatorSelectionProbability(const uint256& validator_id,
                                             const ValidatorNetworkState& state);

/**
 * Select validators for checkpoint
 */
std::vector<uint256> SelectValidatorsForCheckpoint(const ValidatorNetworkState& state,
                                                  int checkpoint_height);

/**
 * Validate validator signature
 */
bool ValidateValidatorSignature(const uint256& validator_id, const uint256& message_hash,
                               const std::vector<uint8_t>& signature);

/**
 * Performance monitoring
 */

/**
 * Monitor hybrid consensus performance
 */
void MonitorHybridConsensusPerformance();

/**
 * Get consensus statistics
 */
struct ConsensusStats {
    uint32_t total_checkpoints;
    uint32_t finalized_blocks;
    double average_finality_time;
    uint32_t active_validators;
    uint64_t total_stake;
    double network_participation_rate;
    
    ConsensusStats() : total_checkpoints(0), finalized_blocks(0), average_finality_time(0.0),
                      active_validators(0), total_stake(0), network_participation_rate(0.0) {}
};

ConsensusStats GetConsensusStatistics();

/**
 * Cleanup and maintenance
 */

/**
 * Cleanup expired checkpoints
 */
void CleanupExpiredCheckpoints(ValidatorNetworkState& state);

/**
 * Update validator reputation scores
 */
void UpdateValidatorReputationScores(ValidatorNetworkState& state);

/**
 * Perform periodic maintenance
 */
void PerformHybridConsensusMaintenance();

} // namespace hybrid
} // namespace consensus

#endif // BITCOIN_CONSENSUS_HYBRID_H
