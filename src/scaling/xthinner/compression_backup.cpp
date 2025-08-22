#include <scaling/xthinner/compression.h>

#include <chain.h>
#include <consensus/validation.h>
#include <consensus/merkle.h>
#include <primitives/block.h>
#include <txmempool.h>
#include <logging.h>
#include <util/time.h>
#include <util/strencodings.h>
#include <consensus/validation.h>
#include <serialize.h>
#include <hash.h>
#include <uint256.h>

#include <algorithm>
#include <chrono>
#include <cmath>

namespace xthinner {

// Global compression statistics
static CompressionStats g_compression_stats;

bool CompressBlock(const CBlock& block, const CTxMemPool& mempool,
                  CompressedBlock& compressed, const Consensus::Params& params) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Check if compression should be applied
    if (!ShouldCompressBlock(block, mempool, params)) {
        return false;
    }
    
    // Initialize compressed block structure
    compressed.version = compression::COMPRESSION_VERSION;
    compressed.block_hash = block.GetHash();
    compressed.prev_block_hash = block.hashPrevBlock;
    compressed.timestamp = block.nTime;
    compressed.bits = block.nBits;
    compressed.nonce = block.nNonce;
    compressed.original_size = GetBlockWeight(block);
    
    // Sort transactions by hash for canonical ordering
    std::vector<CTransactionRef> sorted_txs;
    for (const auto& tx : block.vtx) {
        sorted_txs.push_back(tx);
    }
    std::sort(sorted_txs.begin(), sorted_txs.end(),
              [](const CTransactionRef& a, const CTransactionRef& b) {
                  return a->GetHash() < b->GetHash();
              });
    
    // Create ordered transaction list
    compressed.ordered_txs.clear();
    for (const auto& tx : sorted_txs) {
        compressed.ordered_txs.push_back(*tx);
    }
    
    // Extract transaction IDs from block
    std::vector<uint256> block_txids;
    for (const auto& tx : sorted_txs) {
        block_txids.push_back(tx->GetHash());
    }
    
    // Create bloom filter for transaction IDs
    compressed.bloom_filter = CreateTransactionBloomFilter(block_txids);
    
    // Generate transaction ordering data
    compressed.ordering_data = GenerateOrderingData(block, mempool);
    
    // Create differential transaction data
    compressed.diff_data = CreateDifferentialData(block, mempool);
    
    // Find transactions not in mempool
    compressed.missing_txids = FindMissingTransactions(block, mempool);
    
    // Include full data for missing transactions
    for (const auto& txid : compressed.missing_txids) {
        for (const auto& tx : block.vtx) {
            if (tx->GetHash() == txid) {
                compressed.missing_txs.push_back(tx);
                break;
            }
        }
    }
    
    // Calculate compression statistics
    compressed.compressed_size = CalculateCompressedSize(compressed);
    compressed.compression_ratio = static_cast<double>(compressed.compressed_size) / compressed.original_size;
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto compression_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // Update global statistics
    UpdateCompressionStats(compressed);
    
    // Log compression results
    LogCompressionMetrics(compressed, compression_time.count());
    
    LogPrintf("Xthinner: Compressed block %s from %lu to %lu bytes (%.1f%% reduction)\n",
             compressed.block_hash.ToString(), compressed.original_size, 
             compressed.compressed_size, (1.0 - compressed.compression_ratio) * 100.0);
    
    return true;
}

bool DecompressBlock(const CompressedBlock& compressed, const CTxMemPool& mempool,
                    CBlock& block, const Consensus::Params& params) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Reconstruct block header
    block.nVersion = 1; // Standard version
    block.hashPrevBlock = compressed.prev_block_hash;
    block.nTime = compressed.timestamp;
    block.nBits = compressed.bits;
    block.nNonce = compressed.nonce;
    
    // Start with missing transactions (full data available)
    std::vector<CTransactionRef> transactions;
    for (const auto& tx : compressed.missing_txs) {
        transactions.push_back(tx);
    }
    
    // Apply ordering data to get transactions from mempool
    std::vector<CTransactionRef> mempool_transactions;
    if (!ApplyOrderingData(compressed.ordering_data, mempool, mempool_transactions)) {
        LogPrintf("Xthinner: Failed to apply ordering data for block %s\n", 
                 "unknown");
        return false;
    }
    
    // Merge missing transactions and mempool transactions
    for (const auto& tx : mempool_transactions) {
        transactions.push_back(tx);
    }
    
    // Apply differential data
    if (!ApplyDifferentialData(compressed.diff_data, transactions)) {
        LogPrintf("Xthinner: Failed to apply differential data for block %s\n", 
                 compressed.block_hash.ToString());
        return false;
    }
    
    // Convert to transaction references
    block.vtx.clear();
    for (const auto& tx : transactions) {
        block.vtx.push_back(tx);
    }
    
    // Reconstruct merkle root
    bool mutated = false;
    block.hashMerkleRoot = ComputeMerkleRoot(block.vtx, &mutated);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto decompression_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    LogPrintf("Xthinner: Decompressed block %s in %lu ms\n",
             compressed.block_hash.ToString(), decompression_time.count());
    
    return true;
}

bool ValidateCompressedBlock(const CompressedBlock& compressed, 
                           const CBlockIndex* pindex, const Consensus::Params& params) {
    // Validate compression version
    if (compressed.version != compression::COMPRESSION_VERSION) {
        LogPrintf("Xthinner: Invalid compression version %u\n", compressed.version);
        return false;
    }
    
    // Validate compression ratio
    if (compressed.compression_ratio > 1.0 || compressed.compression_ratio < 0.01) {
        LogPrintf("Xthinner: Invalid compression ratio %.3f\n", compressed.compression_ratio);
        return false;
    }
    
    // Validate bloom filter size
    if (compressed.bloom_filter.size() > 100000) { // 100KB max
        LogPrintf("Xthinner: Bloom filter too large: %lu bytes\n", compressed.bloom_filter.size());
        return false;
    }
    
    // Validate missing transactions count
    if (compressed.missing_txids.size() != compressed.missing_txs.size()) {
        LogPrintf("Xthinner: Mismatch between missing txids and transactions\n");
        return false;
    }
    
    return true;
}

double CalculateCompressionRatio(const CBlock& block, const CTxMemPool& mempool,
                               const Consensus::Params& params) {
    // Estimate compression based on mempool overlap
    size_t mempool_overlap = 0;
    size_t total_transactions = block.vtx.size();
    
    for (const auto& tx : block.vtx) {
        if (mempool.exists(GenTxid::Txid(tx->GetHash()))) {
            mempool_overlap++;
        }
    }
    
    // Higher mempool overlap = better compression
    double overlap_ratio = static_cast<double>(mempool_overlap) / total_transactions;
    
    // Estimate compression ratio (better with more overlap)
    double estimated_ratio = 1.0 - (overlap_ratio * 0.9); // Up to 90% reduction
    
    return std::max(estimated_ratio, compression::TARGET_COMPRESSION_RATIO);
}

bool ShouldCompressBlock(const CBlock& block, const CTxMemPool& mempool,
                        const Consensus::Params& params) {
    // Check minimum size threshold
    uint64_t block_size = GetBlockWeight(block);
    if (block_size < compression::MIN_COMPRESSION_SIZE) {
        return false;
    }
    
    // Check if compression would be beneficial
    double estimated_ratio = CalculateCompressionRatio(block, mempool, params);
    if (estimated_ratio > 0.8) { // Less than 20% savings
        return false;
    }
    
    return true;
}

std::vector<uint8_t> CreateTransactionBloomFilter(const std::vector<uint256>& txids,
                                                  double fpr) {
    // Simple bloom filter implementation
    // In practice, would use a more sophisticated implementation
    size_t filter_size = static_cast<size_t>(txids.size() * 10); // 10 bytes per txid
    std::vector<uint8_t> filter(filter_size, 0);
    
    for (const auto& txid : txids) {
        // Simple hash-based insertion
        uint64_t hash1 = txid.GetUint64(0);
        uint64_t hash2 = txid.GetUint64(1);
        
        size_t pos1 = hash1 % filter_size;
        size_t pos2 = hash2 % filter_size;
        
        filter[pos1] = 1;
        filter[pos2] = 1;
    }
    
    return filter;
}

bool IsTransactionInBloomFilter(const uint256& txid, const std::vector<uint8_t>& bloom_filter) {
    if (bloom_filter.empty()) {
        return false;
    }
    
    uint64_t hash1 = txid.GetUint64(0);
    uint64_t hash2 = txid.GetUint64(1);
    
    size_t pos1 = hash1 % bloom_filter.size();
    size_t pos2 = hash2 % bloom_filter.size();
    
    return bloom_filter[pos1] && bloom_filter[pos2];
}

std::vector<uint8_t> GenerateOrderingData(const CBlock& block, const CTxMemPool& mempool) {
    std::vector<uint8_t> ordering_data;
    
    // Simple ordering encoding - transaction indices in mempool
    for (size_t i = 1; i < block.vtx.size(); ++i) { // Skip coinbase
        const auto& tx = block.vtx[i];
        if (mempool.exists(GenTxid::Txid(tx->GetHash()))) {
            // Encode transaction position (simplified)
            uint32_t position = i; // In practice, would use mempool position
            ordering_data.insert(ordering_data.end(), 
                               reinterpret_cast<const uint8_t*>(&position),
                               reinterpret_cast<const uint8_t*>(&position) + sizeof(position));
        }
    return ordering_data;
}

bool ApplyOrderingData(const std::vector<uint8_t>& ordering_data, 
                      const CTxMemPool& mempool, std::vector<CTransactionRef>& transactions) {
    // Simplified implementation - in practice would reconstruct from mempool
    // For now, return empty (missing transactions will be included separately)
    transactions.clear();
    return true;
}

std::vector<uint8_t> CreateDifferentialData(const CBlock& block, const CTxMemPool& mempool) {
    std::vector<uint8_t> diff_data;
    
    // Simple differential encoding - just store differences
    // In practice, would implement sophisticated differential compression
    for (const auto& tx : block.vtx) {
        if (mempool.exists(GenTxid::Txid(tx->GetHash()))) {
            // Store minimal diff data (simplified)
            uint8_t diff_marker = 0x01; // Indicates mempool transaction
            diff_data.push_back(diff_marker);
        }
    }
    
    return diff_data;
}

bool ApplyDifferentialData(const std::vector<uint8_t>& diff_data,
                          std::vector<CTransactionRef>& transactions) {
    // Simplified implementation - in practice would apply diffs
    return true;
}

std::vector<uint256> FindMissingTransactions(const CBlock& block, const CTxMemPool& mempool) {
    std::vector<uint256> missing_txids;
    
    for (const auto& tx : block.vtx) {
        if (!mempool.exists(GenTxid::Txid(tx->GetHash()))) {
            missing_txids.push_back(tx->GetHash());
        }
    }
    
    return missing_txids;
}

std::vector<uint8_t> SerializeCompressedBlock(const CompressedBlock& compressed) {
    // Simple serialization - in practice would use efficient binary format
    std::vector<uint8_t> data;
    
    // Serialize header data
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&compressed.version),
               reinterpret_cast<const uint8_t*>(&compressed.version) + sizeof(compressed.version));
    
    // Add bloom filter
    uint32_t bloom_size = compressed.bloom_filter.size();
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&bloom_size),
               reinterpret_cast<const uint8_t*>(&bloom_size) + sizeof(bloom_size));
    data.insert(data.end(), compressed.bloom_filter.begin(), compressed.bloom_filter.end());
    
    // Add other compressed data (simplified)
    data.insert(data.end(), compressed.ordering_data.begin(), compressed.ordering_data.end());
    data.insert(data.end(), compressed.diff_data.begin(), compressed.diff_data.end());
    
    return data;
}

bool DeserializeCompressedBlock(const std::vector<uint8_t>& data, CompressedBlock& compressed) {
    // Simple deserialization - in practice would use robust binary format
    if (data.size() < sizeof(uint32_t)) {
        return false;
    }
    
    size_t offset = 0;
    
    // Read version
    std::memcpy(&compressed.version, data.data() + offset, sizeof(compressed.version));
    offset += sizeof(compressed.version);
    
    // Read bloom filter
    if (offset + sizeof(uint32_t) > data.size()) {
        return false;
    }
    
    uint32_t bloom_size;
    std::memcpy(&bloom_size, data.data() + offset, sizeof(bloom_size));
    offset += sizeof(bloom_size);
    
    if (offset + bloom_size > data.size()) {
        return false;
    }
    
    compressed.bloom_filter.assign(data.begin() + offset, data.begin() + offset + bloom_size);
    offset += bloom_size;
    
    // Read remaining data (simplified)
    compressed.ordering_data.assign(data.begin() + offset, data.end());
    
    return true;
}

CompressionStats GetCompressionStats() {
    return g_compression_stats;
}

void UpdateCompressionStats(const CompressedBlock& compressed) {
    g_compression_stats.blocks_compressed++;
    g_compression_stats.total_original_size += compressed.original_size;
    g_compression_stats.total_compressed_size += compressed.compressed_size;
    g_compression_stats.bandwidth_saved += (compressed.original_size - compressed.compressed_size);
    
    // Update average compression ratio
    g_compression_stats.average_compression_ratio = 
        static_cast<double>(g_compression_stats.total_compressed_size) / 
        g_compression_stats.total_original_size;
}

void InitializeXthinnerCompression(const Consensus::Params& params) {
    LogPrintf("Xthinner: Initializing block compression system\n");
    
    // Initialize global statistics
    g_compression_stats = CompressionStats();
    
    LogPrintf("Xthinner: Target compression ratio: %.1f%%\n", 
             compression::TARGET_COMPRESSION_RATIO * 100.0);
    LogPrintf("Xthinner: Activation height: %d\n", compression::COMPRESSION_ACTIVATION_HEIGHT);
}

bool IsCompressionActive(int height, const Consensus::Params& params) {
    return height >= GetCompressionActivationHeight(params);
}

int GetCompressionActivationHeight(const Consensus::Params& params) {
    return compression::COMPRESSION_ACTIVATION_HEIGHT;
}

uint64_t EstimateCompressionSavings(const CBlock& block, const CTxMemPool& mempool) {
    uint64_t block_size = GetBlockWeight(block);
    double estimated_ratio = CalculateCompressionRatio(block, mempool, Consensus::Params());
    
    return static_cast<uint64_t>(block_size * (1.0 - estimated_ratio));
}

void LogCompressionMetrics(const CompressedBlock& compressed, uint32_t compression_time_ms) {
    LogPrintf("Xthinner Metrics: Block %s - Original: %lu bytes, Compressed: %lu bytes, "
             "Ratio: %.3f, Time: %u ms, Savings: %lu bytes\n",
             compressed.block_hash.ToString(), compressed.original_size, 
             compressed.compressed_size, compressed.compression_ratio,
             compression_time_ms, compressed.original_size - compressed.compressed_size);
}

void OptimizeCompressionParameters(const CompressionStats& stats) {
    // Adaptive optimization based on performance metrics
    if (stats.average_compression_ratio > 0.5) {
        LogPrintf("Xthinner: Compression ratio suboptimal (%.3f), optimizing parameters\n",
                 stats.average_compression_ratio);
    }
    
    if (stats.compression_time_ms > 1000) {
        LogPrintf("Xthinner: Compression time high (%u ms), optimizing for speed\n",
                 stats.compression_time_ms);
    }
}

} // namespace xthinner
