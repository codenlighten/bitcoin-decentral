#ifndef BITCOIN_SCALING_MEMPOOL_ADVANCED_H
#define BITCOIN_SCALING_MEMPOOL_ADVANCED_H

#include <cstdint>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <string>
#include <chrono>

class CTransaction;
class CTxMemPool;
class CBlockIndex;
class uint256;
namespace Consensus { struct Params; }

/**
 * Bitcoin Decentral Advanced Mempool Management
 * 
 * This module implements advanced mempool management for Bitcoin Decentral,
 * enabling efficient handling of 1M+ transactions with intelligent prioritization,
 * clustering, and resource optimization.
 * 
 * Key Features:
 * - High-capacity transaction storage (1M+ transactions)
 * - Intelligent transaction prioritization and clustering
 * - Dynamic fee estimation and optimization
 * - Memory-efficient data structures with fast lookup
 * - Transaction dependency graph management
 * - Adaptive eviction policies for memory management
 * - Integration with CTOR, unbounded blocks, and Xthinner compression
 */

namespace mempool {

/**
 * Advanced mempool constants
 */
namespace advanced {
    // Maximum mempool size (8M transactions for large block testing)
    static const size_t MAX_MEMPOOL_TRANSACTIONS = 8000000;
    
    // Maximum mempool memory usage (8GB for large block testing)
    static const size_t MAX_MEMPOOL_MEMORY = 8ULL * 1024 * 1024 * 1024;
    
    // Transaction priority levels
    static const int PRIORITY_LEVELS = 10;
    
    // Clustering window size (transactions)
    static const size_t CLUSTERING_WINDOW = 10000;
    
    // Fee estimation window (blocks)
    static const int FEE_ESTIMATION_WINDOW = 144; // ~24 hours
    
    // Eviction batch size
    static const size_t EVICTION_BATCH_SIZE = 1000;
    
    // Advanced mempool activation height
    static const int ADVANCED_MEMPOOL_ACTIVATION_HEIGHT = 4000;
}

/**
 * Transaction priority classification
 */
enum TransactionPriority {
    PRIORITY_URGENT = 0,        // Highest fee, time-sensitive
    PRIORITY_HIGH = 1,          // High fee transactions
    PRIORITY_NORMAL = 2,        // Standard fee transactions
    PRIORITY_LOW = 3,           // Low fee transactions
    PRIORITY_MINIMAL = 4        // Minimal fee transactions
};

/**
 * Transaction cluster information
 */
struct TransactionCluster {
    uint256 cluster_id;                     // Unique cluster identifier
    std::vector<uint256> transaction_ids;   // Transactions in cluster
    uint64_t total_size;                    // Total cluster size
    uint64_t total_fees;                    // Total cluster fees
    TransactionPriority priority;           // Cluster priority
    std::chrono::time_point<std::chrono::steady_clock> created_time; // Creation time
    
    TransactionCluster() : total_size(0), total_fees(0), priority(PRIORITY_NORMAL) {}
};

/**
 * Advanced transaction entry
 */
struct AdvancedTxEntry {
    std::shared_ptr<const CTransaction> tx; // Transaction data
    uint64_t fee;                          // Transaction fee
    uint64_t size;                         // Transaction size
    TransactionPriority priority;          // Priority level
    uint256 cluster_id;                    // Associated cluster
    std::chrono::time_point<std::chrono::steady_clock> entry_time; // Entry time
    std::set<uint256> dependencies; // Transaction dependencies
    std::set<uint256> dependents;   // Dependent transactions
    
    AdvancedTxEntry() : fee(0), size(0), priority(PRIORITY_NORMAL) {}
};

/**
 * Mempool statistics
 */
struct MempoolStats {
    size_t total_transactions;              // Total transactions
    size_t total_memory_usage;              // Total memory usage
    size_t transactions_by_priority[mempool::advanced::PRIORITY_LEVELS]; // Transactions per priority
    size_t total_clusters;                  // Total clusters
    double average_fee_rate;                // Average fee rate
    uint64_t total_fees;                    // Total fees
    std::chrono::milliseconds avg_processing_time; // Average processing time
    
    MempoolStats() : total_transactions(0), total_memory_usage(0), 
                    total_clusters(0), average_fee_rate(0.0), total_fees(0) {
        for (int i = 0; i < mempool::advanced::PRIORITY_LEVELS; ++i) {
            transactions_by_priority[i] = 0;
        }
    }
};

/**
 * Fee estimation data
 */
struct FeeEstimation {
    double fee_rates[10];      // Fee rates per priority
    uint32_t confirmation_targets[10]; // Target confirmations
    double confidence_intervals[10];   // Confidence levels
    std::chrono::time_point<std::chrono::steady_clock> last_update; // Last update
    
    FeeEstimation() {
        for (int i = 0; i < mempool::advanced::PRIORITY_LEVELS; ++i) {
            fee_rates[i] = 0.0;
            confirmation_targets[i] = 0;
            confidence_intervals[i] = 0.0;
        }
    }
};

/**
 * Core advanced mempool functions
 */

/**
 * Initialize advanced mempool management
 */
bool InitializeAdvancedMempool(const Consensus::Params& params);

/**
 * Add transaction to advanced mempool
 */
bool AddTransactionToAdvancedMempool(const CTransaction& tx, uint64_t fee,
                                   CTxMemPool& mempool, const Consensus::Params& params);

/**
 * Remove transaction from advanced mempool
 */
bool RemoveTransactionFromAdvancedMempool(const uint256& txid, CTxMemPool& mempool);

/**
 * Get transaction priority based on fee and other factors
 */
TransactionPriority CalculateTransactionPriority(const CTransaction& tx, uint64_t fee,
                                                const CTxMemPool& mempool);

/**
 * Create or update transaction cluster
 */
bool UpdateTransactionCluster(const uint256& txid, const AdvancedTxEntry& entry);

/**
 * Get transactions for block template with advanced selection
 */
std::vector<std::shared_ptr<const CTransaction>> 
GetTransactionsForBlockTemplate(size_t max_block_size, const CTxMemPool& mempool,
                               const Consensus::Params& params);

/**
 * Perform intelligent mempool eviction
 */
size_t PerformIntelligentEviction(CTxMemPool& mempool, size_t target_size);

/**
 * Update fee estimation based on recent blocks
 */
void UpdateFeeEstimation(const CBlockIndex* pindex, const Consensus::Params& params);

/**
 * Get estimated fee for target confirmation time
 */
double GetEstimatedFee(uint32_t target_confirmations, TransactionPriority priority);

/**
 * Analyze transaction dependencies
 */
bool AnalyzeTransactionDependencies(const uint256& txid, const CTxMemPool& mempool);

/**
 * Optimize mempool structure for performance
 */
void OptimizeMempoolStructure(CTxMemPool& mempool);

/**
 * Get advanced mempool statistics
 */
MempoolStats GetAdvancedMempoolStats(const CTxMemPool& mempool);

/**
 * Validate mempool consistency
 */
bool ValidateMempoolConsistency(const CTxMemPool& mempool);

/**
 * Handle mempool overflow conditions
 */
bool HandleMempoolOverflow(CTxMemPool& mempool, const Consensus::Params& params);

/**
 * Get transaction cluster information
 */
TransactionCluster GetTransactionCluster(const uint256& cluster_id);

/**
 * Merge compatible transaction clusters
 */
bool MergeTransactionClusters(const uint256& cluster1_id, const uint256& cluster2_id);

/**
 * Check if advanced mempool is active
 */
bool IsAdvancedMempoolActive(int height, const Consensus::Params& params);

/**
 * Get advanced mempool activation height
 */
int GetAdvancedMempoolActivationHeight(const Consensus::Params& params);

/**
 * Monitor mempool performance metrics
 */
void MonitorMempoolPerformance(const CTxMemPool& mempool);

/**
 * Log advanced mempool statistics
 */
void LogAdvancedMempoolStats(const MempoolStats& stats);

/**
 * Cleanup expired transactions and clusters
 */
void CleanupExpiredTransactions(CTxMemPool& mempool);

/**
 * Integrate with CTOR for transaction ordering
 */
std::vector<std::shared_ptr<const CTransaction>>
GetCTOROrderedTransactions(const std::vector<std::shared_ptr<const CTransaction>>& transactions);

/**
 * Integrate with Xthinner for compression optimization
 */
void OptimizeForXthinnerCompression(CTxMemPool& mempool);

/**
 * Handle unbounded block size integration
 */
size_t CalculateOptimalBlockSize(const CTxMemPool& mempool, const Consensus::Params& params);

} // namespace mempool

#endif // BITCOIN_SCALING_MEMPOOL_ADVANCED_H
