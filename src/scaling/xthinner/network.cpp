#include <scaling/xthinner/network.h>
#include <scaling/xthinner/compression.h>

#include <chain.h>
#include <chainparams.h>
#include <consensus/validation.h>
#include <net.h>
#include <net_processing.h>
#include <primitives/block.h>
#include <logging.h>
#include <netmessagemaker.h>
#include <txmempool.h>
#include <util/time.h>
#include <consensus/params.h>

#include <algorithm>
#include <chrono>

namespace xthinner {

// Global network compression settings
static NetworkCompressionSettings g_network_settings;

// Peer compression info map
static std::map<NodeId, PeerCompressionInfo> g_peer_compression_info;

bool SendCompressedBlock(const CompressedBlock& compressed, CNode* peer, CConnman& connman) {
    if (!peer || !PeerSupportsCompression(peer)) {
        LogPrintf("Xthinner: Peer %d does not support compression\n", peer->GetId());
        return false;
    }
    
    // Serialize compressed block
    std::vector<uint8_t> serialized = SerializeCompressedBlock(compressed);
    
    // Create network message
    CNetMsgMaker msgMaker(peer->GetCommonVersion());
    connman.PushMessage(peer, msgMaker.Make("xthinner_block", serialized));
    
    LogPrintf("Xthinner: Sent compressed block %s to peer %d (%lu bytes)\n",
             compressed.block_hash.ToString(), peer->GetId(), serialized.size());
    
    return true;
}

bool RequestCompressedBlock(const uint256& block_hash, CNode* peer, CConnman& connman) {
    if (!peer || !PeerSupportsCompression(peer)) {
        return false;
    }
    
    // Create block request message
    std::vector<uint8_t> request_data;
    request_data.insert(request_data.end(), block_hash.begin(), block_hash.end());
    
    CNetMsgMaker msgMaker(peer->GetCommonVersion());
    connman.PushMessage(peer, msgMaker.Make("xthinner_request", request_data));
    
    LogPrintf("Xthinner: Requested compressed block %s from peer %d\n",
             block_hash.ToString(), peer->GetId());
    
    return true;
}

bool ProcessCompressedBlockMessage(const std::vector<uint8_t>& message, CNode* peer,
                                  const CTxMemPool& mempool, CBlock& block) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Deserialize compressed block
    CompressedBlock compressed;
    if (!DeserializeCompressedBlock(message, compressed)) {
        LogPrintf("Xthinner: Failed to deserialize compressed block from peer %d\n", peer->GetId());
        return false;
    }
    
    // Validate compressed block
    if (!ValidateCompressedBlock(compressed, nullptr, Params().GetConsensus())) {
        LogPrintf("Xthinner: Invalid compressed block from peer %d\n", peer->GetId());
        return false;
    }
    
    // Decompress block
    if (!DecompressBlock(compressed, mempool, block, Params().GetConsensus())) {
        LogPrintf("Xthinner: Failed to decompress block from peer %d\n", peer->GetId());
        return HandleCompressionFailure(compressed.block_hash, peer, 
                                       const_cast<CConnman&>(g_connman));
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto decompression_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // Update peer statistics
    UpdatePeerCompressionStats(peer, compressed, decompression_time.count());
    
    LogPrintf("Xthinner: Successfully decompressed block %s from peer %d in %lu ms\n",
             compressed.block_hash.ToString(), peer->GetId(), decompression_time.count());
    
    return true;
}

bool AnnounceXthinnerCapability(CNode* peer, CConnman& connman) {
    if (!peer) {
        return false;
    }
    
    // Create capability announcement
    std::vector<uint8_t> capability_data;
    
    uint32_t capabilities = XTHINNER_COMPRESSION | XTHINNER_DECOMPRESSION | 
                           XTHINNER_ADAPTIVE | XTHINNER_STATS;
    uint32_t version = compression::COMPRESSION_VERSION;
    
    capability_data.insert(capability_data.end(), 
                          reinterpret_cast<const uint8_t*>(&capabilities),
                          reinterpret_cast<const uint8_t*>(&capabilities) + sizeof(capabilities));
    capability_data.insert(capability_data.end(),
                          reinterpret_cast<const uint8_t*>(&version),
                          reinterpret_cast<const uint8_t*>(&version) + sizeof(version));
    
    CNetMsgMaker msgMaker(peer->GetCommonVersion());
    connman.PushMessage(peer, msgMaker.Make("xthinner_capability", capability_data));
    
    LogPrintf("Xthinner: Announced capability to peer %d\n", peer->GetId());
    
    return true;
}

bool ProcessCapabilityAnnouncement(const std::vector<uint8_t>& message, CNode* peer) {
    if (!peer || message.size() < sizeof(uint32_t) * 2) {
        return false;
    }
    
    PeerCompressionInfo info;
    
    // Parse capability data
    std::memcpy(&info.capabilities, message.data(), sizeof(info.capabilities));
    std::memcpy(&info.version, message.data() + sizeof(info.capabilities), sizeof(info.version));
    
    // Validate capability
    if (!ValidatePeerCapability(peer, info)) {
        return false;
    }
    
    // Store peer compression info
    g_peer_compression_info[peer->GetId()] = info;
    
    LogPrintf("Xthinner: Peer %d announced capabilities: 0x%x, version: %u\n",
             peer->GetId(), info.capabilities, info.version);
    
    return true;
}

CNode* SelectCompressionPeer(const std::vector<CNode*>& peers, const uint256& block_hash) {
    CNode* best_peer = nullptr;
    double best_score = 0.0;
    
    for (CNode* peer : peers) {
        if (!PeerSupportsCompression(peer)) {
            continue;
        }
        
        PeerCompressionInfo info = GetPeerCompressionInfo(peer);
        
        // Calculate peer score based on compression performance
        double score = 1.0 - info.avg_compression_ratio; // Better compression = higher score
        score *= (info.reliable ? 1.0 : 0.5); // Reliability factor
        score *= (10000.0 / std::max(info.avg_compression_time, 1u)); // Speed factor
        
        if (score > best_score) {
            best_score = score;
            best_peer = peer;
        }
    }
    
    if (best_peer) {
        LogPrintf("Xthinner: Selected peer %d for compressed block request (score: %.3f)\n",
                 best_peer->GetId(), best_score);
    }
    
    return best_peer;
}

bool ValidatePeerCapability(CNode* peer, const PeerCompressionInfo& info) {
    // Validate version compatibility
    if (info.version > compression::COMPRESSION_VERSION) {
        LogPrintf("Xthinner: Peer %d has unsupported version %u\n", peer->GetId(), info.version);
        return false;
    }
    
    // Validate capability flags
    if (info.capabilities == 0) {
        LogPrintf("Xthinner: Peer %d has no compression capabilities\n", peer->GetId());
        return false;
    }
    
    return true;
}

void UpdatePeerCompressionStats(CNode* peer, const CompressedBlock& compressed,
                               uint32_t compression_time) {
    if (!peer) {
        return;
    }
    
    auto it = g_peer_compression_info.find(peer->GetId());
    if (it != g_peer_compression_info.end()) {
        PeerCompressionInfo& info = it->second;
        
        // Update running averages
        info.avg_compression_ratio = (info.avg_compression_ratio + compressed.compression_ratio) / 2.0;
        info.avg_compression_time = (info.avg_compression_time + compression_time) / 2;
        
        // Update reliability based on success
        info.reliable = true; // Successful operation
    }
}

NetworkCompressionSettings GetNetworkCompressionSettings() {
    return g_network_settings;
}

void UpdateNetworkCompressionSettings(const NetworkCompressionSettings& settings) {
    g_network_settings = settings;
    
    LogPrintf("Xthinner: Updated network compression settings - enabled: %s, min_ratio: %.3f\n",
             settings.enabled ? "yes" : "no", settings.min_compression_ratio);
}

bool PeerSupportsCompression(CNode* peer) {
    if (!peer) {
        return false;
    }
    
    auto it = g_peer_compression_info.find(peer->GetId());
    if (it == g_peer_compression_info.end()) {
        return false;
    }
    
    return (it->second.capabilities & XTHINNER_COMPRESSION) != 0;
}

PeerCompressionInfo GetPeerCompressionInfo(CNode* peer) {
    if (!peer) {
        return PeerCompressionInfo();
    }
    
    auto it = g_peer_compression_info.find(peer->GetId());
    if (it != g_peer_compression_info.end()) {
        return it->second;
    }
    
    return PeerCompressionInfo();
}

void BroadcastCompressionStats(CConnman& connman) {
    CompressionStats stats = GetCompressionStats();
    
    // Create statistics message
    std::vector<uint8_t> stats_data;
    stats_data.insert(stats_data.end(),
                     reinterpret_cast<const uint8_t*>(&stats.blocks_compressed),
                     reinterpret_cast<const uint8_t*>(&stats.blocks_compressed) + sizeof(stats.blocks_compressed));
    stats_data.insert(stats_data.end(),
                     reinterpret_cast<const uint8_t*>(&stats.average_compression_ratio),
                     reinterpret_cast<const uint8_t*>(&stats.average_compression_ratio) + sizeof(stats.average_compression_ratio));
    
    // Broadcast to all supporting peers
    connman.ForEachNode([&](CNode* peer) {
        if (PeerSupportsCompression(peer)) {
            CNetMsgMaker msgMaker(peer->GetCommonVersion());
            connman.PushMessage(peer, msgMaker.Make("xthinner_stats", stats_data));
        }
    });
    
    LogPrintf("Xthinner: Broadcasted compression statistics to network\n");
}

bool ProcessCompressionStats(const std::vector<uint8_t>& message, CNode* peer) {
    if (!peer || message.size() < sizeof(uint64_t) + sizeof(double)) {
        return false;
    }
    
    uint64_t blocks_compressed;
    double avg_compression_ratio;
    
    std::memcpy(&blocks_compressed, message.data(), sizeof(blocks_compressed));
    std::memcpy(&avg_compression_ratio, message.data() + sizeof(blocks_compressed), sizeof(avg_compression_ratio));
    
    LogPrintf("Xthinner: Peer %d stats - blocks: %lu, avg_ratio: %.3f\n",
             peer->GetId(), blocks_compressed, avg_compression_ratio);
    
    return true;
}

void OptimizeNetworkCompression(const std::vector<CNode*>& peers) {
    // Analyze peer performance and optimize settings
    double total_ratio = 0.0;
    uint32_t total_time = 0;
    int peer_count = 0;
    
    for (CNode* peer : peers) {
        if (PeerSupportsCompression(peer)) {
            PeerCompressionInfo info = GetPeerCompressionInfo(peer);
            total_ratio += info.avg_compression_ratio;
            total_time += info.avg_compression_time;
            peer_count++;
        }
    }
    
    if (peer_count > 0) {
        double avg_ratio = total_ratio / peer_count;
        uint32_t avg_time = total_time / peer_count;
        
        // Optimize settings based on network performance
        NetworkCompressionSettings settings = GetNetworkCompressionSettings();
        
        if (avg_ratio > 0.7) {
            settings.min_compression_ratio = 0.6; // Lower threshold for poor compression
        } else if (avg_ratio < 0.3) {
            settings.min_compression_ratio = 0.4; // Higher threshold for good compression
        }
        
        if (avg_time > 2000) {
            settings.max_compression_time = 3000; // Increase timeout for slow network
        }
        
        UpdateNetworkCompressionSettings(settings);
    }
}

bool HandleCompressionFailure(const uint256& block_hash, CNode* peer, CConnman& connman) {
    LogPrintf("Xthinner: Compression failure for block %s from peer %d, falling back to standard request\n",
             block_hash.ToString(), peer->GetId());
    
    // Mark peer as unreliable for compression
    auto it = g_peer_compression_info.find(peer->GetId());
    if (it != g_peer_compression_info.end()) {
        it->second.reliable = false;
    }
    
    // Request standard block as fallback
    CNetMsgMaker msgMaker(peer->GetCommonVersion());
    connman.PushMessage(peer, msgMaker.Make(NetMsgType::GETDATA, 
                       CInv(MSG_BLOCK, block_hash)));
    
    return true;
}

void MonitorCompressionPerformance() {
    CompressionStats stats = GetCompressionStats();
    
    LogPrintf("Xthinner Performance: blocks=%lu, avg_ratio=%.3f, bandwidth_saved=%lu bytes\n",
             stats.blocks_compressed, stats.average_compression_ratio, stats.bandwidth_saved);
    
    // Optimize parameters if needed
    OptimizeCompressionParameters(stats);
}

void InitializeNetworkCompression(CConnman& connman) {
    LogPrintf("Xthinner: Initializing network compression system\n");
    
    // Initialize default settings
    g_network_settings = NetworkCompressionSettings();
    
    // Clear peer info
    g_peer_compression_info.clear();
    
    LogPrintf("Xthinner: Network compression system initialized\n");
}

void ShutdownNetworkCompression() {
    LogPrintf("Xthinner: Shutting down network compression system\n");
    
    // Clear peer info
    g_peer_compression_info.clear();
    
    // Log final statistics
    MonitorCompressionPerformance();
}

} // namespace xthinner
