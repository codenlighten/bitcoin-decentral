#ifndef BITCOIN_INTEROP_BRIDGES_H
#define BITCOIN_INTEROP_BRIDGES_H

#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <memory>

class CTransaction;
#include <uint256.h>
class CScript;

/**
 * Bitcoin Decentral Cross-Chain Interoperability System
 * 
 * This module implements a comprehensive cross-chain bridge infrastructure
 * that enables secure asset transfers and communication between Bitcoin Decentral
 * and other major blockchain networks.
 * 
 * Key Features:
 * - Secure multi-signature bridges to major blockchains
 * - Atomic swap capabilities for trustless exchanges
 * - Cross-chain state verification and proof systems
 * - Asset wrapping and unwrapping mechanisms
 * - Inter-Blockchain Communication (IBC) protocol support
 * - Decentralized bridge governance and security
 */

namespace interop {
namespace bridges {

/**
 * Cross-chain bridge constants
 */
namespace params {
    // Minimum confirmations for cross-chain transfers
    static const int MIN_CONFIRMATIONS_BITCOIN = 6;
    static const int MIN_CONFIRMATIONS_ETHEREUM = 12;
    static const int MIN_CONFIRMATIONS_DEFAULT = 10;
    
    // Bridge security parameters
    static const int MIN_BRIDGE_VALIDATORS = 7;
    static const int BRIDGE_THRESHOLD = 5; // 5 of 7 multisig
    
    // Asset transfer limits (in satoshis)
    static const uint64_t MIN_TRANSFER_AMOUNT = 100000; // 0.001 BTCD
    static const uint64_t MAX_TRANSFER_AMOUNT = 1000000000000ULL; // 10,000 BTCD
    
    // Bridge fee parameters
    static const double BRIDGE_FEE_RATE = 0.001; // 0.1%
    static const uint64_t MIN_BRIDGE_FEE = 10000; // 0.0001 BTCD
    
    // Timeout parameters (in blocks)
    static const int ATOMIC_SWAP_TIMEOUT = 144; // 24 hours
    static const int BRIDGE_TIMEOUT = 1008; // 1 week
    
    // Supported blockchain networks
    enum SupportedChain {
        CHAIN_BITCOIN = 1,
        CHAIN_ETHEREUM = 2,
        CHAIN_BINANCE_SMART_CHAIN = 3,
        CHAIN_POLYGON = 4,
        CHAIN_AVALANCHE = 5,
        CHAIN_SOLANA = 6,
        CHAIN_CARDANO = 7,
        CHAIN_POLKADOT = 8
    };
}

/**
 * Bridge validator information
 */
struct BridgeValidator {
    uint256 validator_id;           // Unique validator identifier
    uint256 public_key;             // Validator public key
    std::string btcd_address;       // Bitcoin Decentral address
    std::map<params::SupportedChain, std::string> chain_addresses; // Addresses on other chains
    uint64_t stake_amount;          // Staked amount for bridge security
    bool is_active;                 // Whether validator is active
    double reputation_score;        // Validator reputation
    uint32_t total_transfers;       // Total transfers processed
    uint64_t total_volume;          // Total volume processed
    
    BridgeValidator() : stake_amount(0), is_active(false), reputation_score(1.0),
                       total_transfers(0), total_volume(0) {}
};

/**
 * Cross-chain asset information
 */
struct CrossChainAsset {
    uint256 asset_id;               // Unique asset identifier
    params::SupportedChain origin_chain; // Original blockchain
    std::string symbol;             // Asset symbol (e.g., BTC, ETH)
    std::string name;               // Asset name
    uint8_t decimals;               // Number of decimal places
    uint256 contract_address;       // Contract address on origin chain
    uint64_t total_supply;          // Total supply on Bitcoin Decentral
    uint64_t circulating_supply;    // Circulating supply
    bool is_native;                 // Whether native to Bitcoin Decentral
    
    CrossChainAsset() : origin_chain(params::CHAIN_BITCOIN), decimals(8),
                       total_supply(0), circulating_supply(0), is_native(true) {}
};

/**
 * Bridge transfer request
 */
struct BridgeTransfer {
    uint256 transfer_id;            // Unique transfer identifier
    uint256 user_id;                // User initiating transfer
    params::SupportedChain source_chain; // Source blockchain
    params::SupportedChain dest_chain;   // Destination blockchain
    uint256 asset_id;               // Asset being transferred
    uint64_t amount;                // Transfer amount
    std::string source_address;     // Source address
    std::string dest_address;       // Destination address
    uint256 source_tx_hash;         // Source transaction hash
    enum Status : uint8_t {
        TRANSFER_PENDING = 0,       // Transfer initiated, awaiting confirmations
        TRANSFER_CONFIRMED,         // Source transaction confirmed
        TRANSFER_PROCESSING,        // Being processed by bridge validators
        TRANSFER_COMPLETED,         // Transfer completed successfully
        TRANSFER_FAILED,            // Transfer failed
        TRANSFER_REFUNDED          // Transfer refunded to user
    };
    
    uint256 dest_tx_hash;           // Destination transaction hash
    uint32_t creation_height;       // Block height when created
    uint32_t completion_height;     // Block height when completed
    Status status;                  // Current transfer status
    std::vector<uint256> validator_signatures; // Validator signatures
    uint64_t bridge_fee;            // Bridge fee paid
    
    BridgeTransfer() : source_chain(params::CHAIN_BITCOIN), dest_chain(params::CHAIN_BITCOIN),
                      amount(0), creation_height(0), completion_height(0),
                      status(TRANSFER_PENDING), bridge_fee(0) {}
};

/**
 * Atomic swap information
 */
struct AtomicSwap {
    uint256 swap_id;                // Unique swap identifier
    uint256 initiator_id;           // Swap initiator
    uint256 participant_id;         // Swap participant
    params::SupportedChain chain_a; // First blockchain
    params::SupportedChain chain_b; // Second blockchain
    uint256 asset_a_id;             // Asset on chain A
    uint256 asset_b_id;             // Asset on chain B
    uint64_t amount_a;              // Amount on chain A
    uint64_t amount_b;              // Amount on chain B
    enum Status : uint8_t {
        SWAP_INITIATED = 0,         // Swap initiated by first party
        SWAP_PARTICIPATED,          // Second party participated
        SWAP_REDEEMED,              // Swap completed successfully
        SWAP_REFUNDED,              // Swap refunded due to timeout
        SWAP_EXPIRED               // Swap expired
    };
    
    uint256 hash_lock;              // Hash lock for atomic swap
    uint256 secret;                 // Secret for unlocking (when revealed)
    uint32_t timeout_height;        // Timeout block height
    Status status;                  // Current swap status
    uint256 contract_a_hash;        // Contract hash on chain A
    uint256 contract_b_hash;        // Contract hash on chain B
    
    AtomicSwap() : chain_a(params::CHAIN_BITCOIN), chain_b(params::CHAIN_BITCOIN),
                  amount_a(0), amount_b(0), timeout_height(0), status(SWAP_INITIATED) {}
};

/**
 * Bridge network state
 */
struct BridgeNetworkState {
    std::map<uint256, BridgeValidator> active_validators;    // Active bridge validators
    std::map<uint256, CrossChainAsset> supported_assets;    // Supported cross-chain assets
    std::map<uint256, BridgeTransfer> active_transfers;     // Active bridge transfers
    std::map<uint256, AtomicSwap> active_swaps;            // Active atomic swaps
    uint64_t total_bridge_volume;                          // Total bridge volume
    uint32_t total_transfers;                              // Total transfers processed
    uint32_t successful_transfers;                         // Successful transfers
    
    BridgeNetworkState() : total_bridge_volume(0), total_transfers(0), successful_transfers(0) {}
};

/**
 * Core bridge functions
 */

/**
 * Initialize cross-chain bridge system
 */
bool InitializeBridgeSystem();

/**
 * Register bridge validator
 */
bool RegisterBridgeValidator(const uint256& validator_id, const uint256& public_key,
                           const std::string& btcd_address, uint64_t stake_amount);

/**
 * Add supported blockchain
 */
bool AddSupportedBlockchain(params::SupportedChain chain, const std::string& rpc_endpoint,
                          const std::string& bridge_contract_address);

/**
 * Register cross-chain asset
 */
bool RegisterCrossChainAsset(const CrossChainAsset& asset);

/**
 * Bridge transfer functions
 */

/**
 * Initiate bridge transfer
 */
bool InitiateBridgeTransfer(const uint256& user_id, params::SupportedChain source_chain,
                          params::SupportedChain dest_chain, const uint256& asset_id,
                          uint64_t amount, const std::string& dest_address);

/**
 * Process bridge transfer
 */
bool ProcessBridgeTransfer(const uint256& transfer_id);

/**
 * Validate bridge transfer
 */
bool ValidateBridgeTransfer(const BridgeTransfer& transfer);

/**
 * Complete bridge transfer
 */
bool CompleteBridgeTransfer(const uint256& transfer_id, const uint256& dest_tx_hash);

/**
 * Refund failed bridge transfer
 */
bool RefundBridgeTransfer(const uint256& transfer_id, const std::string& reason);

/**
 * Get bridge transfer status
 */
BridgeTransfer::Status GetBridgeTransferStatus(const uint256& transfer_id);

/**
 * Atomic swap functions
 */

/**
 * Initiate atomic swap
 */
bool InitiateAtomicSwap(const uint256& initiator_id, params::SupportedChain chain_a,
                       params::SupportedChain chain_b, const uint256& asset_a_id,
                       const uint256& asset_b_id, uint64_t amount_a, uint64_t amount_b,
                       const uint256& hash_lock, uint32_t timeout_height);

/**
 * Participate in atomic swap
 */
bool ParticipateAtomicSwap(const uint256& swap_id, const uint256& participant_id);

/**
 * Redeem atomic swap
 */
bool RedeemAtomicSwap(const uint256& swap_id, const uint256& secret);

/**
 * Refund atomic swap
 */
bool RefundAtomicSwap(const uint256& swap_id);

/**
 * Validate atomic swap parameters
 */
bool ValidateAtomicSwapParameters(const AtomicSwap& swap);

/**
 * Cross-chain verification functions
 */

/**
 * Verify transaction on external chain
 */
bool VerifyExternalChainTransaction(params::SupportedChain chain, const uint256& tx_hash,
                                  uint32_t required_confirmations);

/**
 * Generate cross-chain proof
 */
std::vector<uint8_t> GenerateCrossChainProof(params::SupportedChain chain,
                                           const uint256& tx_hash, uint32_t block_height);

/**
 * Verify cross-chain proof
 */
bool VerifyCrossChainProof(params::SupportedChain chain, const std::vector<uint8_t>& proof,
                         const uint256& tx_hash);

/**
 * Get chain block height
 */
uint32_t GetChainBlockHeight(params::SupportedChain chain);

/**
 * Asset management functions
 */

/**
 * Wrap external asset
 */
bool WrapExternalAsset(const uint256& asset_id, uint64_t amount, const std::string& user_address);

/**
 * Unwrap asset to external chain
 */
bool UnwrapAssetToExternalChain(const uint256& asset_id, uint64_t amount,
                               params::SupportedChain dest_chain, const std::string& dest_address);

/**
 * Calculate bridge fee
 */
uint64_t CalculateBridgeFee(uint64_t amount, params::SupportedChain source_chain,
                          params::SupportedChain dest_chain);

/**
 * Get required confirmations for chain
 */
uint32_t GetRequiredConfirmations(params::SupportedChain chain);

/**
 * Select bridge validators
 */
std::vector<uint256> SelectBridgeValidators();

/**
 * Execute transfer on destination chain
 */
bool ExecuteDestinationChainTransfer(const BridgeTransfer& transfer);

/**
 * Update asset supply
 */
bool UpdateAssetSupply(const uint256& asset_id, uint64_t new_supply);

/**
 * Bridge security functions
 */

/**
 * Validate bridge validator signatures
 */
bool ValidateBridgeValidatorSignatures(const std::vector<uint256>& signatures,
                                     const uint256& message_hash);

/**
 * Check bridge security threshold
 */
bool CheckBridgeSecurityThreshold(const std::vector<uint256>& validator_signatures);

/**
 * Slash malicious bridge validator
 */
bool SlashBridgeValidator(const uint256& validator_id, double penalty_percentage,
                        const std::string& reason);

/**
 * Monitor bridge validator performance
 */
void MonitorBridgeValidatorPerformance();

/**
 * IBC protocol functions
 */

/**
 * Initialize IBC connection
 */
bool InitializeIBCConnection(params::SupportedChain chain, const std::string& connection_id);

/**
 * Create IBC channel
 */
bool CreateIBCChannel(const std::string& connection_id, const std::string& port_id);

/**
 * Send IBC packet
 */
bool SendIBCPacket(const std::string& channel_id, const std::vector<uint8_t>& data,
                  uint64_t timeout_height);

/**
 * Receive IBC packet
 */
bool ReceiveIBCPacket(const std::string& channel_id, const std::vector<uint8_t>& packet_data,
                     const std::vector<uint8_t>& proof);

/**
 * Acknowledge IBC packet
 */
bool AcknowledgeIBCPacket(const std::string& channel_id, uint64_t sequence,
                        const std::vector<uint8_t>& acknowledgement);

/**
 * Utility functions
 */

/**
 * Get bridge network state
 */
BridgeNetworkState GetBridgeNetworkState();

/**
 * Update bridge network state
 */
bool UpdateBridgeNetworkState(const BridgeNetworkState& state);

/**
 * Get supported chains
 */
std::vector<params::SupportedChain> GetSupportedChains();

/**
 * Get chain information
 */
std::string GetChainName(params::SupportedChain chain);
std::string GetChainSymbol(params::SupportedChain chain);
uint8_t GetChainDecimals(params::SupportedChain chain);

/**
 * Convert between chain formats
 */
std::string ConvertAddressFormat(const std::string& address, params::SupportedChain from_chain,
                               params::SupportedChain to_chain);

/**
 * Statistics and monitoring
 */

/**
 * Bridge statistics
 */
struct BridgeStats {
    uint32_t total_validators;
    uint32_t active_transfers;
    uint32_t completed_transfers;
    uint32_t failed_transfers;
    uint64_t total_volume;
    uint64_t total_fees_collected;
    double success_rate;
    double average_transfer_time;
    
    BridgeStats() : total_validators(0), active_transfers(0), completed_transfers(0),
                   failed_transfers(0), total_volume(0), total_fees_collected(0),
                   success_rate(0.0), average_transfer_time(0.0) {}
};

/**
 * Get bridge statistics
 */
BridgeStats GetBridgeStatistics();

/**
 * Monitor bridge performance
 */
void MonitorBridgePerformance();

/**
 * Cleanup expired transfers and swaps
 */
void CleanupExpiredBridgeOperations();

/**
 * Perform bridge maintenance
 */
void PerformBridgeMaintenance();

} // namespace bridges
} // namespace interop

#endif // BITCOIN_INTEROP_BRIDGES_H
