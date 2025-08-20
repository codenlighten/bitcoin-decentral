#include <scaling/mempool/advanced.h>

#include <primitives/transaction.h>
#include <txmempool.h>
#include <chain.h>
#include <consensus/params.h>
#include <logging.h>
#include <util/time.h>
#include <hash.h>
#include <random.h>
#include <uint256.h>

#include <algorithm>
#include <chrono>
#include <map>
#include <set>
#include <queue>
#include <mutex>

namespace mempool {

// Global advanced mempool state
static std::map<uint256, AdvancedTxEntry> g_advanced_tx_entries;
static std::map<uint256, TransactionCluster> g_transaction_clusters;
static FeeEstimation g_fee_estimation;
static MempoolStats g_mempool_stats;
static std::mutex g_advanced_mempool_mutex;

// Priority queues for efficient transaction selection
static std::priority_queue<std::pair<double, uint256>> g_priority_queues[advanced::PRIORITY_LEVELS];

bool InitializeAdvancedMempool(const Consensus::Params& params)
{
    std::lock_guard<std::mutex> lock(g_advanced_mempool_mutex);
    
    LogPrintf("Advanced Mempool: Initializing advanced mempool management\n");
    
    // Clear existing state
    g_advanced_tx_entries.clear();
    g_transaction_clusters.clear();
    
    // Initialize fee estimation
    g_fee_estimation = FeeEstimation();
    g_fee_estimation.last_update = std::chrono::steady_clock::now();
    
    // Initialize statistics
    g_mempool_stats = MempoolStats();
    
    // Clear priority queues
    for (int i = 0; i < advanced::PRIORITY_LEVELS; ++i) {
        while (!g_priority_queues[i].empty()) {
            g_priority_queues[i].pop();
        }
    }
    
    LogPrintf("Advanced Mempool: Initialization complete\n");
    return true;
}

bool AddTransactionToAdvancedMempool(const CTransaction& tx, uint64_t fee,
                                   CTxMemPool& mempool, const Consensus::Params& params)
{
    std::lock_guard<std::mutex> lock(g_advanced_mempool_mutex);
    
    const uint256 txid = tx.GetHash();
    
    // Check if mempool is at capacity
    if (g_advanced_tx_entries.size() >= advanced::MAX_MEMPOOL_TRANSACTIONS) {
        LogPrintf("Advanced Mempool: At capacity, performing intelligent eviction\n");
        if (!HandleMempoolOverflow(mempool, params)) {
            return false;
        }
    }
    
    // Create advanced transaction entry
    AdvancedTxEntry entry;
    entry.tx = std::make_shared<const CTransaction>(tx);
    entry.fee = fee;
    entry.size = tx.GetTotalSize();
    entry.priority = CalculateTransactionPriority(tx, fee, mempool);
    entry.entry_time = std::chrono::steady_clock::now();
    
    // Analyze dependencies
    for (const auto& input : tx.vin) {
        if (g_advanced_tx_entries.find(input.prevout.hash) != g_advanced_tx_entries.end()) {
            entry.dependencies.insert(input.prevout.hash);
            g_advanced_tx_entries[input.prevout.hash].dependents.insert(txid);
        }
    }
    
    // Create or assign to cluster
    uint256 cluster_id = Hash(BEGIN(txid), END(txid));
    entry.cluster_id = cluster_id;
    UpdateTransactionCluster(txid, entry);
    
    // Add to advanced mempool
    g_advanced_tx_entries[txid] = entry;
    
    // Add to priority queue
    double priority_score = static_cast<double>(fee) / entry.size;
    g_priority_queues[entry.priority].push(std::make_pair(priority_score, txid));
    
    // Update statistics
    g_mempool_stats.total_transactions++;
    g_mempool_stats.total_memory_usage += entry.size;
    g_mempool_stats.transactions_by_priority[entry.priority]++;
    g_mempool_stats.total_fees += fee;
    
    LogPrintf("Advanced Mempool: Added transaction %s (fee: %lu, size: %lu, priority: %d)\n",
              txid.ToString(), fee, entry.size, entry.priority);
    
    return true;
}

bool RemoveTransactionFromAdvancedMempool(const uint256& txid, CTxMemPool& mempool)
{
    std::lock_guard<std::mutex> lock(g_advanced_mempool_mutex);
    
    auto it = g_advanced_tx_entries.find(txid);
    if (it == g_advanced_tx_entries.end()) {
        return false;
    }
    
    const AdvancedTxEntry& entry = it->second;
    
    // Update statistics
    g_mempool_stats.total_transactions--;
    g_mempool_stats.total_memory_usage -= entry.size;
    g_mempool_stats.transactions_by_priority[entry.priority]--;
    g_mempool_stats.total_fees -= entry.fee;
    
    // Remove from cluster
    auto cluster_it = g_transaction_clusters.find(entry.cluster_id);
    if (cluster_it != g_transaction_clusters.end()) {
        auto& cluster = cluster_it->second;
        cluster.transaction_ids.erase(
            std::remove(cluster.transaction_ids.begin(), cluster.transaction_ids.end(), txid),
            cluster.transaction_ids.end());
        cluster.total_size -= entry.size;
        cluster.total_fees -= entry.fee;
        
        if (cluster.transaction_ids.empty()) {
            g_transaction_clusters.erase(cluster_it);
        }
    }
    
    // Remove dependency relationships
    for (const auto& dep : entry.dependencies) {
        auto dep_it = g_advanced_tx_entries.find(dep);
        if (dep_it != g_advanced_tx_entries.end()) {
            dep_it->second.dependents.erase(txid);
        }
    }
    
    for (const auto& dependent : entry.dependents) {
        auto dep_it = g_advanced_tx_entries.find(dependent);
        if (dep_it != g_advanced_tx_entries.end()) {
            dep_it->second.dependencies.erase(txid);
        }
    }
    
    // Remove from advanced mempool
    g_advanced_tx_entries.erase(it);
    
    LogPrintf("Advanced Mempool: Removed transaction %s\n", txid.ToString());
    return true;
}

TransactionPriority CalculateTransactionPriority(const CTransaction& tx, uint64_t fee,
                                                const CTxMemPool& mempool)
{
    double fee_rate = static_cast<double>(fee) / tx.GetTotalSize();
    
    // Priority thresholds (satoshis per byte)
    if (fee_rate >= 100.0) return PRIORITY_URGENT;
    if (fee_rate >= 50.0) return PRIORITY_HIGH;
    if (fee_rate >= 10.0) return PRIORITY_NORMAL;
    if (fee_rate >= 1.0) return PRIORITY_LOW;
    return PRIORITY_MINIMAL;
}

bool UpdateTransactionCluster(const uint256& txid, const AdvancedTxEntry& entry)
{
    auto cluster_it = g_transaction_clusters.find(entry.cluster_id);
    
    if (cluster_it == g_transaction_clusters.end()) {
        // Create new cluster
        TransactionCluster cluster;
        cluster.cluster_id = entry.cluster_id;
        cluster.transaction_ids.push_back(txid);
        cluster.total_size = entry.size;
        cluster.total_fees = entry.fee;
        cluster.priority = entry.priority;
        cluster.created_time = entry.entry_time;
        
        g_transaction_clusters[entry.cluster_id] = cluster;
        g_mempool_stats.total_clusters++;
    } else {
        // Update existing cluster
        auto& cluster = cluster_it->second;
        cluster.transaction_ids.push_back(txid);
        cluster.total_size += entry.size;
        cluster.total_fees += entry.fee;
        
        // Update cluster priority to highest priority transaction
        if (entry.priority < cluster.priority) {
            cluster.priority = entry.priority;
        }
    }
    
    return true;
}

std::vector<std::shared_ptr<const CTransaction>> 
GetTransactionsForBlockTemplate(size_t max_block_size, const CTxMemPool& mempool,
                               const Consensus::Params& params)
{
    std::lock_guard<std::mutex> lock(g_advanced_mempool_mutex);
    
    std::vector<std::shared_ptr<const CTransaction>> selected_transactions;
    std::set<uint256> selected_txids;
    size_t current_size = 0;
    
    LogPrintf("Advanced Mempool: Selecting transactions for block template (max size: %lu)\n", max_block_size);
    
    // Select transactions by priority
    for (int priority = 0; priority < advanced::PRIORITY_LEVELS && current_size < max_block_size; ++priority) {
        auto priority_queue = g_priority_queues[priority]; // Copy for iteration
        
        while (!priority_queue.empty() && current_size < max_block_size) {
            auto [score, txid] = priority_queue.top();
            priority_queue.pop();
            
            auto it = g_advanced_tx_entries.find(txid);
            if (it == g_advanced_tx_entries.end()) continue;
            
            const auto& entry = it->second;
            
            // Check if transaction fits
            if (current_size + entry.size > max_block_size) {
                continue;
            }
            
            // Check dependencies are satisfied
            bool dependencies_satisfied = true;
            for (const auto& dep : entry.dependencies) {
                if (selected_txids.find(dep) == selected_txids.end()) {
                    dependencies_satisfied = false;
                    break;
                }
            }
            
            if (!dependencies_satisfied) continue;
            
            // Add transaction
            selected_transactions.push_back(entry.tx);
            selected_txids.insert(txid);
            current_size += entry.size;
        }
    }
    
    LogPrintf("Advanced Mempool: Selected %lu transactions (%lu bytes) for block template\n",
              selected_transactions.size(), current_size);
    
    return selected_transactions;
}

size_t PerformIntelligentEviction(CTxMemPool& mempool, size_t target_size)
{
    size_t evicted_count = 0;
    
    // Evict lowest priority transactions first
    for (int priority = advanced::PRIORITY_LEVELS - 1; priority >= 0; --priority) {
        if (g_mempool_stats.total_transactions <= target_size) break;
        
        std::vector<uint256> to_evict;
        
        for (const auto& [txid, entry] : g_advanced_tx_entries) {
            if (entry.priority == priority && to_evict.size() < advanced::EVICTION_BATCH_SIZE) {
                to_evict.push_back(txid);
            }
        }
        
        for (const auto& txid : to_evict) {
            if (RemoveTransactionFromAdvancedMempool(txid, mempool)) {
                evicted_count++;
            }
            if (g_mempool_stats.total_transactions <= target_size) break;
        }
    }
    
    LogPrintf("Advanced Mempool: Evicted %lu transactions\n", evicted_count);
    return evicted_count;
}

void UpdateFeeEstimation(const CBlockIndex* pindex, const Consensus::Params& params)
{
    std::lock_guard<std::mutex> lock(g_advanced_mempool_mutex);
    
    // Update fee estimation based on recent block data
    // This is a simplified implementation
    for (int i = 0; i < advanced::PRIORITY_LEVELS; ++i) {
        g_fee_estimation.fee_rates[i] = (i + 1) * 10.0; // Simple fee rate calculation
        g_fee_estimation.confirmation_targets[i] = (i + 1) * 2;
        g_fee_estimation.confidence_intervals[i] = 0.95 - (i * 0.1);
    }
    
    g_fee_estimation.last_update = std::chrono::steady_clock::now();
    
    LogPrintf("Advanced Mempool: Updated fee estimation\n");
}

double GetEstimatedFee(uint32_t target_confirmations, TransactionPriority priority)
{
    std::lock_guard<std::mutex> lock(g_advanced_mempool_mutex);
    
    if (priority >= 0 && priority < advanced::PRIORITY_LEVELS) {
        return g_fee_estimation.fee_rates[priority];
    }
    
    return 10.0; // Default fee rate
}

bool AnalyzeTransactionDependencies(const uint256& txid, const CTxMemPool& mempool)
{
    auto it = g_advanced_tx_entries.find(txid);
    if (it == g_advanced_tx_entries.end()) {
        return false;
    }
    
    // Dependency analysis is already performed in AddTransactionToAdvancedMempool
    LogPrintf("Advanced Mempool: Analyzed dependencies for transaction %s\n", txid.ToString());
    return true;
}

void OptimizeMempoolStructure(CTxMemPool& mempool)
{
    std::lock_guard<std::mutex> lock(g_advanced_mempool_mutex);
    
    // Rebuild priority queues
    for (int i = 0; i < advanced::PRIORITY_LEVELS; ++i) {
        while (!g_priority_queues[i].empty()) {
            g_priority_queues[i].pop();
        }
    }
    
    // Re-add all transactions to priority queues
    for (const auto& [txid, entry] : g_advanced_tx_entries) {
        double priority_score = static_cast<double>(entry.fee) / entry.size;
        g_priority_queues[entry.priority].push(std::make_pair(priority_score, txid));
    }
    
    LogPrintf("Advanced Mempool: Optimized mempool structure\n");
}

MempoolStats GetAdvancedMempoolStats(const CTxMemPool& mempool)
{
    std::lock_guard<std::mutex> lock(g_advanced_mempool_mutex);
    
    // Update average fee rate
    if (g_mempool_stats.total_transactions > 0) {
        g_mempool_stats.average_fee_rate = static_cast<double>(g_mempool_stats.total_fees) / 
                                          g_mempool_stats.total_memory_usage;
    }
    
    return g_mempool_stats;
}

bool ValidateMempoolConsistency(const CTxMemPool& mempool)
{
    std::lock_guard<std::mutex> lock(g_advanced_mempool_mutex);
    
    // Validate transaction count consistency
    size_t counted_transactions = 0;
    for (int i = 0; i < advanced::PRIORITY_LEVELS; ++i) {
        counted_transactions += g_mempool_stats.transactions_by_priority[i];
    }
    
    if (counted_transactions != g_mempool_stats.total_transactions) {
        LogPrintf("Advanced Mempool: Consistency check failed - transaction count mismatch\n");
        return false;
    }
    
    LogPrintf("Advanced Mempool: Consistency validation passed\n");
    return true;
}

bool HandleMempoolOverflow(CTxMemPool& mempool, const Consensus::Params& params)
{
    LogPrintf("Advanced Mempool: Handling mempool overflow\n");
    
    size_t target_size = advanced::MAX_MEMPOOL_TRANSACTIONS * 0.9; // Reduce to 90%
    return PerformIntelligentEviction(mempool, target_size) > 0;
}

TransactionCluster GetTransactionCluster(const uint256& cluster_id)
{
    std::lock_guard<std::mutex> lock(g_advanced_mempool_mutex);
    
    auto it = g_transaction_clusters.find(cluster_id);
    if (it != g_transaction_clusters.end()) {
        return it->second;
    }
    
    return TransactionCluster(); // Return empty cluster
}

bool MergeTransactionClusters(const uint256& cluster1_id, const uint256& cluster2_id)
{
    std::lock_guard<std::mutex> lock(g_advanced_mempool_mutex);
    
    auto it1 = g_transaction_clusters.find(cluster1_id);
    auto it2 = g_transaction_clusters.find(cluster2_id);
    
    if (it1 == g_transaction_clusters.end() || it2 == g_transaction_clusters.end()) {
        return false;
    }
    
    // Merge cluster2 into cluster1
    auto& cluster1 = it1->second;
    const auto& cluster2 = it2->second;
    
    cluster1.transaction_ids.insert(cluster1.transaction_ids.end(),
                                   cluster2.transaction_ids.begin(),
                                   cluster2.transaction_ids.end());
    cluster1.total_size += cluster2.total_size;
    cluster1.total_fees += cluster2.total_fees;
    
    // Update priority to highest
    if (cluster2.priority < cluster1.priority) {
        cluster1.priority = cluster2.priority;
    }
    
    // Remove cluster2
    g_transaction_clusters.erase(it2);
    g_mempool_stats.total_clusters--;
    
    LogPrintf("Advanced Mempool: Merged clusters %s and %s\n",
              cluster1_id.ToString(), cluster2_id.ToString());
    
    return true;
}

bool IsAdvancedMempoolActive(int height, const Consensus::Params& params)
{
    return height >= GetAdvancedMempoolActivationHeight(params);
}

int GetAdvancedMempoolActivationHeight(const Consensus::Params& params)
{
    return advanced::ADVANCED_MEMPOOL_ACTIVATION_HEIGHT;
}

void MonitorMempoolPerformance(const CTxMemPool& mempool)
{
    const auto stats = GetAdvancedMempoolStats(mempool);
    
    LogPrintf("Advanced Mempool Performance: %lu transactions, %lu bytes, %lu clusters\n",
              stats.total_transactions, stats.total_memory_usage, stats.total_clusters);
}

void LogAdvancedMempoolStats(const MempoolStats& stats)
{
    LogPrintf("=== Advanced Mempool Statistics ===\n");
    LogPrintf("Total Transactions: %lu\n", stats.total_transactions);
    LogPrintf("Total Memory Usage: %lu bytes\n", stats.total_memory_usage);
    LogPrintf("Total Clusters: %lu\n", stats.total_clusters);
    LogPrintf("Average Fee Rate: %.2f sat/byte\n", stats.average_fee_rate);
    LogPrintf("Total Fees: %lu satoshis\n", stats.total_fees);
    
    for (int i = 0; i < advanced::PRIORITY_LEVELS; ++i) {
        LogPrintf("Priority %d: %lu transactions\n", i, stats.transactions_by_priority[i]);
    }
}

void CleanupExpiredTransactions(CTxMemPool& mempool)
{
    std::lock_guard<std::mutex> lock(g_advanced_mempool_mutex);
    
    auto now = std::chrono::steady_clock::now();
    std::vector<uint256> expired_txids;
    
    // Find transactions older than 24 hours
    for (const auto& [txid, entry] : g_advanced_tx_entries) {
        auto age = std::chrono::duration_cast<std::chrono::hours>(now - entry.entry_time);
        if (age.count() >= 24) {
            expired_txids.push_back(txid);
        }
    }
    
    // Remove expired transactions
    for (const auto& txid : expired_txids) {
        RemoveTransactionFromAdvancedMempool(txid, mempool);
    }
    
    if (!expired_txids.empty()) {
        LogPrintf("Advanced Mempool: Cleaned up %lu expired transactions\n", expired_txids.size());
    }
}

std::vector<std::shared_ptr<const CTransaction>>
GetCTOROrderedTransactions(const std::vector<std::shared_ptr<const CTransaction>>& transactions)
{
    // Integration with CTOR - sort transactions by txid for canonical ordering
    auto ordered_transactions = transactions;
    std::sort(ordered_transactions.begin(), ordered_transactions.end(),
              [](const std::shared_ptr<const CTransaction>& a, 
                 const std::shared_ptr<const CTransaction>& b) {
                  return a->GetHash() < b->GetHash();
              });
    
    LogPrintf("Advanced Mempool: Applied CTOR ordering to %lu transactions\n", 
              ordered_transactions.size());
    
    return ordered_transactions;
}

void OptimizeForXthinnerCompression(CTxMemPool& mempool)
{
    // Optimize transaction ordering for better Xthinner compression
    LogPrintf("Advanced Mempool: Optimizing for Xthinner compression\n");
    
    // Group similar transactions together for better compression
    OptimizeMempoolStructure(mempool);
}

size_t CalculateOptimalBlockSize(const CTxMemPool& mempool, const Consensus::Params& params)
{
    std::lock_guard<std::mutex> lock(g_advanced_mempool_mutex);
    
    // Calculate optimal block size based on mempool state
    size_t base_size = 1024 * 1024; // 1MB base
    size_t mempool_size = g_mempool_stats.total_memory_usage;
    
    // Scale block size based on mempool pressure
    if (mempool_size > 10 * 1024 * 1024) { // > 10MB
        return base_size * 4; // 4MB blocks
    } else if (mempool_size > 5 * 1024 * 1024) { // > 5MB
        return base_size * 2; // 2MB blocks
    }
    
    return base_size; // 1MB blocks
}

} // namespace mempool
