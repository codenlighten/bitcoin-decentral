#ifndef BITCOIN_CONSENSUS_GOVERNANCE_H
#define BITCOIN_CONSENSUS_GOVERNANCE_H

#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <memory>

class CTransaction;
#include <uint256.h>
namespace consensus { namespace hybrid { struct ValidatorNetworkState; } }

/**
 * Bitcoin Decentral On-Chain Governance System
 * 
 * This module implements a comprehensive on-chain governance system that allows
 * validators and stakeholders to propose, vote on, and execute protocol upgrades
 * and parameter changes through decentralized consensus.
 * 
 * Key Features:
 * - Decentralized proposal creation and voting
 * - Stake-weighted voting mechanism
 * - Automatic execution of approved proposals
 * - Parameter adjustment capabilities
 * - Protocol upgrade coordination
 * - Emergency governance procedures
 */

namespace consensus {
namespace governance {

/**
 * Governance constants
 */
namespace params {
    // Minimum stake required to create a proposal (in satoshis)
    static const uint64_t MIN_PROPOSAL_STAKE = 10000 * 100000000ULL; // 10,000 BTCD
    
    // Voting period duration (in blocks)
    static const int VOTING_PERIOD_BLOCKS = 2016; // ~2 weeks
    
    // Minimum participation rate for valid proposal
    static const double MIN_PARTICIPATION_RATE = 0.33; // 33%
    
    // Approval threshold for regular proposals
    static const double APPROVAL_THRESHOLD = 0.67; // 67%
    
    // Approval threshold for critical proposals
    static const double CRITICAL_APPROVAL_THRESHOLD = 0.80; // 80%
    
    // Maximum number of active proposals
    static const int MAX_ACTIVE_PROPOSALS = 10;
    
    // Proposal execution delay (in blocks)
    static const int EXECUTION_DELAY_BLOCKS = 144; // ~1 day
    
    // Emergency proposal fast-track threshold
    static const double EMERGENCY_THRESHOLD = 0.90; // 90%
}

/**
 * Proposal types
 */
enum ProposalType {
    PROPOSAL_PARAMETER_CHANGE,      // Change consensus parameters
    PROPOSAL_PROTOCOL_UPGRADE,      // Protocol version upgrade
    PROPOSAL_VALIDATOR_MANAGEMENT,  // Validator network changes
    PROPOSAL_EMERGENCY_ACTION,      // Emergency network actions
    PROPOSAL_GOVERNANCE_CHANGE,     // Governance rule changes
    PROPOSAL_CUSTOM                 // Custom proposal type
};

/**
 * Proposal status
 */
enum ProposalStatus {
    PROPOSAL_PENDING,       // Proposal created, voting not started
    PROPOSAL_ACTIVE,        // Voting in progress
    PROPOSAL_APPROVED,      // Proposal approved, awaiting execution
    PROPOSAL_REJECTED,      // Proposal rejected by vote
    PROPOSAL_EXECUTED,      // Proposal successfully executed
    PROPOSAL_FAILED,        // Proposal execution failed
    PROPOSAL_EXPIRED        // Proposal expired without sufficient votes
};

/**
 * Governance proposal
 */
struct GovernanceProposal {
    uint256 proposal_id;            // Unique proposal identifier
    uint256 creator_id;             // Proposal creator (validator ID)
    ProposalType type;              // Type of proposal
    std::string title;              // Proposal title
    std::string description;        // Detailed description
    std::vector<uint8_t> data;      // Proposal-specific data
    uint32_t creation_height;       // Block height when created
    uint32_t voting_start_height;   // Voting start height
    uint32_t voting_end_height;     // Voting end height
    uint32_t execution_height;      // Scheduled execution height
    ProposalStatus status;          // Current proposal status
    uint64_t required_stake;        // Required stake for this proposal
    bool is_critical;               // Whether this is a critical proposal
    bool is_emergency;              // Whether this is an emergency proposal
    
    // Voting results
    uint64_t votes_for;             // Total stake voting for
    uint64_t votes_against;         // Total stake voting against
    uint64_t votes_abstain;         // Total stake abstaining
    uint32_t voter_count;           // Number of validators who voted
    
    GovernanceProposal() : type(PROPOSAL_CUSTOM), creation_height(0), voting_start_height(0),
                          voting_end_height(0), execution_height(0), status(PROPOSAL_PENDING),
                          required_stake(0), is_critical(false), is_emergency(false),
                          votes_for(0), votes_against(0), votes_abstain(0), voter_count(0) {}
};

/**
 * Vote information
 */
struct VoteInfo {
    uint256 proposal_id;            // Proposal being voted on
    uint256 validator_id;           // Validator casting vote
    enum VoteChoice {
        VOTE_FOR,
        VOTE_AGAINST,
        VOTE_ABSTAIN
    } choice;                       // Vote choice
    uint64_t stake_weight;          // Stake weight at time of vote
    uint32_t vote_height;           // Block height when vote was cast
    std::string reason;             // Optional vote reason
    
    VoteInfo() : choice(VOTE_ABSTAIN), stake_weight(0), vote_height(0) {}
};

/**
 * Governance state
 */
struct GovernanceState {
    std::map<uint256, GovernanceProposal> active_proposals;     // Active proposals
    std::map<uint256, GovernanceProposal> completed_proposals;  // Completed proposals
    std::map<uint256, std::vector<VoteInfo>> proposal_votes;    // Votes by proposal
    uint32_t current_epoch;                                     // Current governance epoch
    uint32_t total_proposals_created;                           // Total proposals ever created
    uint32_t total_proposals_executed;                          // Total proposals executed
    
    GovernanceState() : current_epoch(0), total_proposals_created(0), total_proposals_executed(0) {}
};

/**
 * Parameter change specification
 */
struct ParameterChange {
    std::string parameter_name;     // Name of parameter to change
    std::string old_value;          // Current parameter value
    std::string new_value;          // Proposed new value
    std::string justification;      // Justification for change
};

/**
 * Core governance functions
 */

/**
 * Initialize governance system
 */
bool InitializeGovernanceSystem();

/**
 * Create new governance proposal
 */
bool CreateProposal(const uint256& creator_id, ProposalType type,
                   const std::string& title, const std::string& description,
                   const std::vector<uint8_t>& data, bool is_critical = false);

/**
 * Submit vote on proposal
 */
bool SubmitVote(const uint256& proposal_id, const uint256& validator_id,
               VoteInfo::VoteChoice choice, const std::string& reason = "");

/**
 * Process governance transaction
 */
bool ProcessGovernanceTransaction(const CTransaction& tx);

/**
 * Update proposal status
 */
bool UpdateProposalStatus(const uint256& proposal_id, int current_height);

/**
 * Execute approved proposal
 */
bool ExecuteProposal(const uint256& proposal_id);

/**
 * Check if proposal meets approval threshold
 */
bool CheckApprovalThreshold(const GovernanceProposal& proposal);

/**
 * Calculate participation rate for proposal
 */
double CalculateParticipationRate(const GovernanceProposal& proposal,
                                 const consensus::hybrid::ValidatorNetworkState& validator_state);

/**
 * Proposal management functions
 */

/**
 * Get proposal by ID
 */
GovernanceProposal* GetProposal(const uint256& proposal_id);

/**
 * Get all active proposals
 */
std::vector<GovernanceProposal> GetActiveProposals();

/**
 * Get proposal votes
 */
std::vector<VoteInfo> GetProposalVotes(const uint256& proposal_id);

/**
 * Check if validator can create proposal
 */
bool CanCreateProposal(const uint256& validator_id, ProposalType type);

/**
 * Check if validator can vote on proposal
 */
bool CanVoteOnProposal(const uint256& validator_id, const uint256& proposal_id);

/**
 * Get validator voting power
 */
uint64_t GetValidatorVotingPower(const uint256& validator_id);

/**
 * Parameter change functions
 */

/**
 * Create parameter change proposal
 */
bool CreateParameterChangeProposal(const uint256& creator_id,
                                  const std::vector<ParameterChange>& changes,
                                  const std::string& justification);

/**
 * Execute parameter changes
 */
bool ExecuteParameterChanges(const std::vector<ParameterChange>& changes);

/**
 * Validate parameter change
 */
bool ValidateParameterChange(const ParameterChange& change);

/**
 * Get current parameter value
 */
std::string GetParameterValue(const std::string& parameter_name);

/**
 * Set parameter value
 */
bool SetParameterValue(const std::string& parameter_name, const std::string& value);

/**
 * Protocol upgrade functions
 */

/**
 * Create protocol upgrade proposal
 */
bool CreateProtocolUpgradeProposal(const uint256& creator_id,
                                  uint32_t new_version, uint32_t activation_height,
                                  const std::string& upgrade_description);

/**
 * Execute protocol upgrade
 */
bool ExecuteProtocolUpgrade(uint32_t new_version, uint32_t activation_height);

/**
 * Check protocol upgrade compatibility
 */
bool CheckProtocolUpgradeCompatibility(uint32_t new_version);

/**
 * Emergency governance functions
 */

/**
 * Create emergency proposal
 */
bool CreateEmergencyProposal(const uint256& creator_id,
                           const std::string& emergency_description,
                           const std::vector<uint8_t>& emergency_data);

/**
 * Check if emergency conditions are met
 */
bool CheckEmergencyConditions();

/**
 * Execute emergency action
 */
bool ExecuteEmergencyAction(const std::vector<uint8_t>& emergency_data);

/**
 * Activate emergency governance mode
 */
bool ActivateEmergencyGovernance();

/**
 * Deactivate emergency governance mode
 */
bool DeactivateEmergencyGovernance();

/**
 * Utility functions
 */

/**
 * Get governance state
 */
GovernanceState GetGovernanceState();

/**
 * Update governance state
 */
bool UpdateGovernanceState(const GovernanceState& state);

/**
 * Calculate proposal hash
 */
uint256 CalculateProposalHash(const GovernanceProposal& proposal);

/**
 * Validate proposal data
 */
bool ValidateProposalData(const GovernanceProposal& proposal);

/**
 * Check proposal conflicts
 */
bool CheckProposalConflicts(const GovernanceProposal& proposal);

/**
 * Get proposal execution cost
 */
uint64_t GetProposalExecutionCost(const GovernanceProposal& proposal);

/**
 * Governance statistics
 */
struct GovernanceStats {
    uint32_t total_proposals;
    uint32_t active_proposals;
    uint32_t executed_proposals;
    uint32_t rejected_proposals;
    double average_participation_rate;
    uint32_t total_voters;
    uint64_t total_voting_stake;
    
    GovernanceStats() : total_proposals(0), active_proposals(0), executed_proposals(0),
                       rejected_proposals(0), average_participation_rate(0.0),
                       total_voters(0), total_voting_stake(0) {}
};

/**
 * Get governance statistics
 */
GovernanceStats GetGovernanceStatistics();

/**
 * Monitor governance activity
 */
void MonitorGovernanceActivity();

/**
 * Cleanup expired proposals
 */
void CleanupExpiredProposals();

/**
 * Perform governance maintenance
 */
void PerformGovernanceMaintenance();

/**
 * Governance event logging
 */
void LogProposalCreated(const GovernanceProposal& proposal);
void LogVoteCast(const VoteInfo& vote);
void LogProposalExecuted(const GovernanceProposal& proposal);
void LogProposalRejected(const GovernanceProposal& proposal);

} // namespace governance
} // namespace consensus

#endif // BITCOIN_CONSENSUS_GOVERNANCE_H
