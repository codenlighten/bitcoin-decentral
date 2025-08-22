#ifndef BITCOIN_SCALING_XTHINNER_NETWORK_H
#define BITCOIN_SCALING_XTHINNER_NETWORK_H

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <net.h>                // CNode, CConnman, NodeId
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <txmempool.h>
#include <uint256.h>
#include <scaling/xthinner/compression.h>   // defines CompressedBlock

class CBlock;
class CTransaction;
class CBlockIndex;
class CTxMemPool;
class CNode;
class CConnman;
namespace Consensus { struct Params; }

/**
 * Bitcoin Decentral Xthinner Network Integration
 * 
 * This module provides network integration for Xthinner compression,
 * enabling efficient block propagation with 90%+ bandwidth reduction
 * across the Bitcoin Decentral network.
 */

namespace xthinner {

/**
 * Network message types for Xthinner
 */
enum XthinnerMessageType {
    XTHINNER_BLOCK_REQUEST = 1,     // Request compressed block
    XTHINNER_BLOCK_RESPONSE = 2,    // Compressed block data
    XTHINNER_CAPABILITY = 3,        // Node capability announcement
    XTHINNER_STATS = 4              // Compression statistics
};

/**
 * Xthinner capability flags
 */
enum XthinnerCapability {
    XTHINNER_COMPRESSION = 0x01,    // Supports block compression
    XTHINNER_DECOMPRESSION = 0x02,  // Supports block decompression
    XTHINNER_ADAPTIVE = 0x04,       // Supports adaptive compression
    XTHINNER_CAP_STATS = 0x08       // Supports statistics sharing
};

/**
 * Network compression settings
 */
struct NetworkCompressionSettings {
    bool enabled;                   // Compression enabled
    double min_compression_ratio;   // Minimum ratio to use compression
    uint32_t max_compression_time;  // Maximum compression time (ms)
    uint32_t peer_timeout;          // Peer response timeout (ms)
    bool adaptive_mode;             // Adaptive compression based on network
    
    NetworkCompressionSettings() : enabled(true), min_compression_ratio(0.5),
                                  max_compression_time(5000), peer_timeout(30000),
                                  adaptive_mode(true) {}
};

/**
 * Peer compression capability
 */
struct PeerCompressionInfo {
    uint32_t capabilities;          // Capability flags
    uint32_t version;              // Xthinner version
    double avg_compression_ratio;   // Average compression ratio achieved
    uint32_t avg_compression_time;  // Average compression time
    bool reliable;                  // Peer reliability for compression
    
    PeerCompressionInfo() : capabilities(0), version(0), avg_compression_ratio(1.0),
                           avg_compression_time(0), reliable(true) {}
};

/**
 * Core network functions
 */

/**
 * Send compressed block to peer
 */
bool SendCompressedBlock(const CompressedBlock& compressed, CNode* peer, CConnman& connman);

/**
 * Request compressed block from peer
 */
bool RequestCompressedBlock(const uint256& block_hash, CNode* peer, CConnman& connman);

/**
 * Handle compressed block message from peer
 */
bool ProcessCompressedBlockMessage(const std::vector<uint8_t>& message, CNode* peer,
                                  const CTxMemPool& mempool, CBlock& block);

/**
 * Announce Xthinner capability to peer
 */
bool AnnounceXthinnerCapability(CNode* peer, CConnman& connman);

/**
 * Process Xthinner capability announcement
 */
bool ProcessCapabilityAnnouncement(const std::vector<uint8_t>& message, CNode* peer);

/**
 * Select best peer for compressed block request
 */
CNode* SelectCompressionPeer(const std::vector<CNode*>& peers, const uint256& block_hash);

/**
 * Validate peer compression capability
 */
bool ValidatePeerCapability(CNode* peer, const PeerCompressionInfo& info);

/**
 * Update peer compression statistics
 */
void UpdatePeerCompressionStats(CNode* peer, const CompressedBlock& compressed,
                               uint32_t compression_time);

/**
 * Get network compression settings
 */
NetworkCompressionSettings GetNetworkCompressionSettings();

/**
 * Update network compression settings
 */
void UpdateNetworkCompressionSettings(const NetworkCompressionSettings& settings);

/**
 * Check if peer supports Xthinner compression
 */
bool PeerSupportsCompression(CNode* peer);

/**
 * Get peer compression info
 */
PeerCompressionInfo GetPeerCompressionInfo(CNode* peer);

/**
 * Broadcast compression statistics to network
 */
void BroadcastCompressionStats(CConnman& connman);

/**
 * Process compression statistics from peer
 */
bool ProcessCompressionStats(const std::vector<uint8_t>& message, CNode* peer);

/**
 * Optimize network compression based on peer feedback
 */
void OptimizeNetworkCompression(const std::vector<CNode*>& peers);

/**
 * Handle compression failure with fallback
 */
bool HandleCompressionFailure(const uint256& block_hash, CNode* peer, CConnman& connman);

/**
 * Monitor network compression performance
 */
void MonitorCompressionPerformance();

/**
 * Initialize network compression system
 */
void InitializeNetworkCompression(CConnman& connman);

/**
 * Shutdown network compression system
 */
void ShutdownNetworkCompression();

} // namespace xthinner

#endif // BITCOIN_SCALING_XTHINNER_NETWORK_H
