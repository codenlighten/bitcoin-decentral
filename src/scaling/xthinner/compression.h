#ifndef BITCOIN_SCALING_XTHINNER_COMPRESSION_H
#define BITCOIN_SCALING_XTHINNER_COMPRESSION_H

#include <cstdint>
#include <vector>
#include <string>
#include <uint256.h>
#include <memory>
#include <primitives/transaction.h>
#include <primitives/block.h>

class CBlock;
class CTransaction;
class CBlockIndex;
class CTxMemPool;
namespace Consensus { struct Params; }

/**
 * Bitcoin Decentral Xthinner Block Compression
 * 
 * This module implements Xthinner compression technology for Bitcoin Decentral,
 * achieving 90%+ bandwidth reduction through advanced block compression techniques.
 * 
 * Key Features:
 * - Ultra-efficient block compression using transaction ordering and mempool state
 * - Differential encoding for transaction data
 * - Bloom filter optimization for transaction identification
 * - Fast decompression with minimal CPU overhead
 * - Backward compatibility with standard block propagation
 * - Adaptive compression based on network conditions
 */

namespace xthinner {

/**
 * Xthinner compression constants
 */
namespace compression {
    // Target compression ratio (90% reduction = 0.1 ratio)
    static const double TARGET_COMPRESSION_RATIO = 0.1;
    
    // Minimum block size for compression (bytes)
    static const uint64_t MIN_COMPRESSION_SIZE = 10000;
    
    // Maximum compression buffer size
    static const uint64_t MAX_COMPRESSION_BUFFER = 100 * 1024 * 1024; // 100MB
    
    // Bloom filter false positive rate
    static const double BLOOM_FILTER_FPR = 0.001; // 0.1%
    
    // Transaction ordering window size
    static const int ORDERING_WINDOW_SIZE = 1000;
    
    // Compression activation height
    static const int COMPRESSION_ACTIVATION_HEIGHT = 3000;
    
    // Compression version
    static const uint32_t COMPRESSION_VERSION = 1;
}

/**
 * Compressed block structure
 */
struct CompressedBlock {
    uint32_t version;                    // Compression version
    uint256 block_hash;                  // Original block hash
    uint256 prev_block_hash;             // Previous block hash
    uint32_t timestamp;                  // Block timestamp
    uint32_t bits;                       // Difficulty bits
    uint32_t nonce;                      // Block nonce
    
    // Compression data
    std::vector<uint8_t> bloom_filter;   // Bloom filter for transaction IDs
    std::vector<uint8_t> ordering_data;  // Transaction ordering information
    std::vector<uint8_t> diff_data;      // Differential transaction data
    std::vector<uint256> missing_txids;  // Transactions not in mempool
    std::vector<CTransaction> missing_txs; // Full missing transactions
    
    // Compression statistics
    uint64_t original_size;              // Original block size
    uint64_t compressed_size;            // Compressed size
    double compression_ratio;            // Achieved compression ratio
    
    CompressedBlock() : version(compression::COMPRESSION_VERSION),
                       original_size(0), compressed_size(0), compression_ratio(1.0) {}
};

/**
 * Compression statistics
 */
struct CompressionStats {
    uint64_t blocks_compressed;          // Total blocks compressed
    uint64_t total_original_size;        // Total original size
    uint64_t total_compressed_size;      // Total compressed size
    double average_compression_ratio;    // Average compression ratio
    uint64_t bandwidth_saved;            // Total bandwidth saved
    uint32_t compression_time_ms;        // Average compression time
    uint32_t decompression_time_ms;      // Average decompression time
    
    CompressionStats() : blocks_compressed(0), total_original_size(0),
                        total_compressed_size(0), average_compression_ratio(1.0),
                        bandwidth_saved(0), compression_time_ms(0), decompression_time_ms(0) {}
};

/**
 * Core compression functions
 */

/**
 * Compress a block using Xthinner algorithm
 */
bool CompressBlock(const CBlock& block, const CTxMemPool& mempool,
                  CompressedBlock& compressed, const Consensus::Params& params);

/**
 * Decompress a compressed block
 */
bool DecompressBlock(const CompressedBlock& compressed, const CTxMemPool& mempool,
                    CBlock& block, const Consensus::Params& params);

/**
 * Validate compressed block integrity
 */
bool ValidateCompressedBlock(const CompressedBlock& compressed, 
                           const CBlockIndex* pindex, const Consensus::Params& params);

/**
 * Calculate compression ratio for a block
 */
double CalculateCompressionRatio(const CBlock& block, const CTxMemPool& mempool,
                               const Consensus::Params& params);

/**
 * Check if block should be compressed
 */
bool ShouldCompressBlock(const CBlock& block, const CTxMemPool& mempool,
                        const Consensus::Params& params);

/**
 * Create bloom filter for transaction IDs
 */
std::vector<uint8_t> CreateTransactionBloomFilter(const std::vector<uint256>& txids,
                                                  double fpr = compression::BLOOM_FILTER_FPR);

/**
 * Check if transaction ID is in bloom filter
 */
bool IsTransactionInBloomFilter(const uint256& txid, const std::vector<uint8_t>& bloom_filter);

/**
 * Generate transaction ordering data
 */
std::vector<uint8_t> GenerateOrderingData(const CBlock& block, const CTxMemPool& mempool);

/**
 * Apply transaction ordering from compressed data
 */
bool ApplyOrderingData(const std::vector<uint8_t>& ordering_data, 
                      const CTxMemPool& mempool, std::vector<CTransaction>& transactions);

/**
 * Create differential transaction data
 */
std::vector<uint8_t> CreateDifferentialData(const CBlock& block, const CTxMemPool& mempool);

/**
 * Apply differential data to reconstruct transactions
 */
bool ApplyDifferentialData(const std::vector<uint8_t>& diff_data,
                          std::vector<CTransaction>& transactions);

/**
 * Find missing transactions not in mempool
 */
std::vector<uint256> FindMissingTransactions(const CBlock& block, const CTxMemPool& mempool);

/**
 * Serialize compressed block for network transmission
 */
std::vector<uint8_t> SerializeCompressedBlock(const CompressedBlock& compressed);

/**
 * Deserialize compressed block from network data
 */
bool DeserializeCompressedBlock(const std::vector<uint8_t>& data, CompressedBlock& compressed);

/**
 * Get compression statistics
 */
CompressionStats GetCompressionStats();

/**
 * Update compression statistics
 */
void UpdateCompressionStats(const CompressedBlock& compressed);

/**
 * Initialize Xthinner compression system
 */
void InitializeXthinnerCompression(const Consensus::Params& params);

/**
 * Check if Xthinner compression is active
 */
bool IsCompressionActive(int height, const Consensus::Params& params);

/**
 * Get compression activation height
 */
int GetCompressionActivationHeight(const Consensus::Params& params);

/**
 * Estimate compression savings for a block
 */
uint64_t EstimateCompressionSavings(const CBlock& block, const CTxMemPool& mempool);

/**
 * Log compression performance metrics
 */
void LogCompressionMetrics(const CompressedBlock& compressed, uint32_t compression_time_ms);

/**
 * Optimize compression parameters based on network conditions
 */
void OptimizeCompressionParameters(const CompressionStats& stats);

} // namespace xthinner

#endif // BITCOIN_SCALING_XTHINNER_COMPRESSION_H
