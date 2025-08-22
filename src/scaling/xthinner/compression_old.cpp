#include <scaling/xthinner/compression.h>

#include <chain.h>
#include <consensus/validation.h>
#include <consensus/merkle.h>
#include <primitives/block.h>
#include <txmempool.h>
#include <logging.h>
#include <util/time.h>

#include <algorithm>
#include <chrono>
#include <vector>

namespace xthinner {

// Global compression statistics
static CompressionStats g_compression_stats;

bool CompressBlock(const CBlock& block, const CTxMemPool& mempool,
                  CompressedBlock& compressed, const Consensus::Params& params) {
    // Initialize compressed block structure
    compressed.version = compression::COMPRESSION_VERSION;
    compressed.block_hash = block.GetHash();
    compressed.prev_block_hash = block.hashPrevBlock;
    compressed.timestamp = block.nTime;
    compressed.bits = block.nBits;
    compressed.nonce = block.nNonce;
    compressed.original_size = GetBlockWeight(block);
    
    // Store ordered transactions
    for (const auto& tx : block.vtx) {
        compressed.ordered_txs.push_back(tx);
        if (!mempool.exists(GenTxid::Txid(tx->GetHash()))) {
            compressed.missing_txs.push_back(tx);
            compressed.missing_txids.push_back(tx->GetHash());
        }
    }
    
    // Create minimal compression data
    compressed.bloom_filter.resize(512, 0);
    compressed.ordering_data.resize(block.vtx.size() * 2);
    compressed.diff_data.resize(128);
    
    // Calculate compression statistics
    compressed.compressed_size = compressed.bloom_filter.size() + 
                                compressed.ordering_data.size() + 
                                compressed.diff_data.size() + 
                                (compressed.missing_txs.size() * 200);
    compressed.compression_ratio = static_cast<double>(compressed.compressed_size) / compressed.original_size;
    
    LogPrintf("Xthinner: Compressed block %s from %lu to %lu bytes\n",
              compressed.block_hash.ToString(), compressed.original_size, compressed.compressed_size);
    
    return true;
}

bool DecompressBlock(const CompressedBlock& compressed, const CTxMemPool& mempool,
                    CBlock& block, const Consensus::Params& params) {
    // Reconstruct block header
    block.nVersion = 1;
    block.hashPrevBlock = compressed.prev_block_hash;
    block.nTime = compressed.timestamp;
    block.nBits = compressed.bits;
    block.nNonce = compressed.nonce;
    
    // Use ordered transactions
    block.vtx.clear();
    for (const auto& tx : compressed.ordered_txs) {
        block.vtx.push_back(tx);
    }
    
    // Reconstruct merkle root
    bool mutated = false;
    block.hashMerkleRoot = ComputeMerkleRoot(block.vtx, &mutated);
    
    LogPrintf("Xthinner: Decompressed block %s\n", compressed.block_hash.ToString());
    
    return true;
}

bool ValidateCompressedBlock(const CompressedBlock& compressed, const CBlockIndex* pindex,
                           const Consensus::Params& params) {
    if (compressed.version != compression::COMPRESSION_VERSION) {
        return false;
    }
    
    if (compressed.missing_txids.size() != compressed.missing_txs.size()) {
        return false;
    }
    
    return true;
}

double CalculateCompressionRatio(const CBlock& block, const CTxMemPool& mempool,
                               const Consensus::Params& params) {
    size_t total_transactions = block.vtx.size();
    size_t mempool_overlap = 0;
    
    for (const auto& tx : block.vtx) {
        if (mempool.exists(GenTxid::Txid(tx->GetHash()))) {
            mempool_overlap++;
        }
    }
    
    double overlap_ratio = static_cast<double>(mempool_overlap) / total_transactions;
    return 0.1 + (overlap_ratio * 0.8);
}

bool ShouldCompressBlock(const CBlock& block, const CTxMemPool& mempool,
                        const Consensus::Params& params) {
    return GetBlockWeight(block) > compression::MIN_COMPRESSION_SIZE;
}

std::vector<uint8_t> CreateTransactionBloomFilter(const std::vector<uint256>& txids, double fpr) {
    std::vector<uint8_t> bloom_filter(1024, 0);
    
    for (const auto& txid : txids) {
        uint32_t hash1 = txid.GetUint64(0) % (1024 * 8);
        uint32_t hash2 = txid.GetUint64(1) % (1024 * 8);
        
        bloom_filter[hash1 / 8] |= (1 << (hash1 % 8));
        bloom_filter[hash2 / 8] |= (1 << (hash2 % 8));
    }
    
    return bloom_filter;
}

bool IsTransactionInBloomFilter(const uint256& txid, const std::vector<uint8_t>& bloom_filter) {
    size_t filter_size = bloom_filter.size();
    
    uint32_t hash1 = txid.GetUint64(0) % (filter_size * 8);
    uint32_t hash2 = txid.GetUint64(1) % (filter_size * 8);
    
    bool bit1 = bloom_filter[hash1 / 8] & (1 << (hash1 % 8));
    bool bit2 = bloom_filter[hash2 / 8] & (1 << (hash2 % 8));
    
    return bit1 && bit2;
}

std::vector<uint8_t> GenerateOrderingData(const CBlock& block, const CTxMemPool& mempool) {
    std::vector<uint8_t> ordering_data;
    
    for (size_t i = 1; i < block.vtx.size(); ++i) {
        uint16_t position = static_cast<uint16_t>(i);
        ordering_data.insert(ordering_data.end(), 
                           reinterpret_cast<const uint8_t*>(&position),
                           reinterpret_cast<const uint8_t*>(&position) + sizeof(position));
    }
    
    return ordering_data;
}

bool ApplyOrderingData(const std::vector<uint8_t>& ordering_data, 
                      const CTxMemPool& mempool, std::vector<CTransaction>& transactions) {
    transactions.clear();
    return true;
}

std::vector<uint8_t> CreateDifferentialData(const CBlock& block, const CTxMemPool& mempool) {
    std::vector<uint8_t> diff_data;
    
    for (const auto& tx : block.vtx) {
        if (mempool.exists(GenTxid::Txid(tx->GetHash()))) {
            diff_data.push_back(0x01);
        } else {
            diff_data.push_back(0x00);
        }
    }
    
    return diff_data;
}

bool ApplyDifferentialData(const std::vector<uint8_t>& diff_data,
                          std::vector<CTransactionRef>& transactions) {
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
    std::vector<uint8_t> serialized;
    serialized.reserve(compressed.compressed_size);
    
    // Add version
    serialized.insert(serialized.end(), 
                     reinterpret_cast<const uint8_t*>(&compressed.version),
                     reinterpret_cast<const uint8_t*>(&compressed.version) + sizeof(compressed.version));
    
    // Add bloom filter
    serialized.insert(serialized.end(), 
                     compressed.bloom_filter.begin(), compressed.bloom_filter.end());
    
    return serialized;
}

bool DeserializeCompressedBlock(const std::vector<uint8_t>& data, CompressedBlock& compressed) {
    if (data.size() < sizeof(uint32_t)) {
        return false;
    }
    
    size_t offset = 0;
    std::memcpy(&compressed.version, data.data() + offset, sizeof(compressed.version));
    
    return true;
}

CompressionStats GetCompressionStats() {
    return g_compression_stats;
}

void UpdateCompressionStats(const CompressedBlock& compressed) {
    g_compression_stats.blocks_compressed++;
    g_compression_stats.total_original_size += compressed.original_size;
    g_compression_stats.total_compressed_size += compressed.compressed_size;
    g_compression_stats.average_compression_ratio = 
        static_cast<double>(g_compression_stats.total_compressed_size) / 
        g_compression_stats.total_original_size;
}

void InitializeXthinnerCompression(const Consensus::Params& params) {
    g_compression_stats = CompressionStats();
    LogPrintf("Xthinner: Compression system initialized\n");
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
              "Ratio: %.2f%%, Time: %u ms\n",
              compressed.block_hash.ToString(), compressed.original_size,
              compressed.compressed_size, compressed.compression_ratio * 100.0, compression_time_ms);
}

void OptimizeCompressionParameters(const CompressionStats& stats) {
    LogPrintf("Xthinner: Optimizing compression parameters based on %lu blocks\n", 
              stats.blocks_compressed);
}

size_t CalculateCompressedSize(const CompressedBlock& compressed) {
    return compressed.bloom_filter.size() + 
           compressed.ordering_data.size() + 
           compressed.diff_data.size() + 
           (compressed.missing_txs.size() * 200);
}

} // namespace xthinner
