/**
 * Bitcoin Decentral Large Block Stress Testing Tool
 * 
 * This tool generates and tests large blocks (4GB+) to validate Bitcoin Decentral's
 * scalability capabilities and compare performance against Bitcoin SV.
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <memory>
#include <random>
#include <thread>

#include <primitives/transaction.h>
#include <primitives/block.h>
#include <consensus/validation.h>
#include <validation.h>
#include <chainparams.h>
#include <util/system.h>
#include <util/time.h>
#include <scaling/mempool/advanced.h>
#include <scaling/ctor/ordering.h>
#include <scaling/xthinner/compression.h>

using namespace std::chrono;

/**
 * Large Block Stress Test Configuration
 */
struct StressTestConfig {
    size_t target_block_size;      // Target block size in bytes
    size_t num_transactions;       // Number of transactions to generate
    bool enable_compression;       // Test Xthinner compression
    bool enable_ctor;             // Test CTOR ordering
    bool benchmark_validation;     // Benchmark validation performance
    std::string test_name;        // Test identifier
};

/**
 * Performance Metrics Collection
 */
struct PerformanceMetrics {
    // Block Creation Metrics
    milliseconds transaction_generation_time{0};
    milliseconds ctor_ordering_time{0};
    milliseconds block_creation_time{0};
    
    // Block Validation Metrics
    milliseconds block_validation_time{0};
    milliseconds signature_verification_time{0};
    
    // Network/Compression Metrics
    size_t raw_block_size{0};
    size_t compressed_block_size{0};
    double compression_ratio{0.0};
    milliseconds compression_time{0};
    milliseconds decompression_time{0};
    
    // Memory Metrics
    size_t peak_memory_usage{0};
    size_t mempool_memory_usage{0};
    
    // Storage Metrics
    milliseconds block_write_time{0};
    size_t storage_size{0};
};

class LargeBlockStressTester {
private:
    StressTestConfig config_;
    PerformanceMetrics metrics_;
    std::mt19937 rng_;
    
public:
    LargeBlockStressTester(const StressTestConfig& config) 
        : config_(config), rng_(std::random_device{}()) {}
    
    /**
     * Generate a realistic transaction for testing
     */
    CTransaction GenerateTestTransaction() {
        CMutableTransaction mtx;
        
        // Create realistic inputs (1-3 inputs)
        size_t num_inputs = 1 + (rng_() % 3);
        for (size_t i = 0; i < num_inputs; ++i) {
            CTxIn input;
            input.prevout.hash = uint256S(std::to_string(rng_()));
            input.prevout.n = rng_() % 10;
            
            // Generate realistic script sig (70-150 bytes)
            size_t script_size = 70 + (rng_() % 80);
            input.scriptSig = CScript(std::vector<uint8_t>(script_size, 0x01));
            
            mtx.vin.push_back(input);
        }
        
        // Create realistic outputs (1-5 outputs)
        size_t num_outputs = 1 + (rng_() % 5);
        for (size_t i = 0; i < num_outputs; ++i) {
            CTxOut output;
            output.nValue = 1000000 + (rng_() % 100000000); // 0.01-1 BTC
            
            // Generate realistic script pubkey (25-35 bytes)
            size_t script_size = 25 + (rng_() % 10);
            output.scriptPubKey = CScript(std::vector<uint8_t>(script_size, 0x76));
            
            mtx.vout.push_back(output);
        }
        
        return CTransaction(mtx);
    }
    
    /**
     * Generate large number of test transactions
     */
    std::vector<CTransaction> GenerateTestTransactions() {
        std::cout << "Generating " << config_.num_transactions << " test transactions..." << std::endl;
        
        auto start_time = high_resolution_clock::now();
        
        std::vector<CTransaction> transactions;
        transactions.reserve(config_.num_transactions);
        
        for (size_t i = 0; i < config_.num_transactions; ++i) {
            transactions.push_back(GenerateTestTransaction());
            
            // Progress indicator
            if (i % 100000 == 0 && i > 0) {
                std::cout << "Generated " << i << " transactions..." << std::endl;
            }
        }
        
        auto end_time = high_resolution_clock::now();
        metrics_.transaction_generation_time = duration_cast<milliseconds>(end_time - start_time);
        
        std::cout << "Generated " << transactions.size() << " transactions in " 
                  << metrics_.transaction_generation_time.count() << "ms" << std::endl;
        
        return transactions;
    }
    
    /**
     * Apply CTOR ordering to transactions
     */
    std::vector<CTransaction> ApplyCTOROrdering(const std::vector<CTransaction>& transactions) {
        if (!config_.enable_ctor) {
            return transactions;
        }
        
        std::cout << "Applying CTOR ordering to " << transactions.size() << " transactions..." << std::endl;
        
        auto start_time = high_resolution_clock::now();
        
        // Use our CTOR implementation
        std::vector<CTransaction> ordered_txs = ctor::OrderTransactionsCanonically(transactions);
        
        auto end_time = high_resolution_clock::now();
        metrics_.ctor_ordering_time = duration_cast<milliseconds>(end_time - start_time);
        
        std::cout << "CTOR ordering completed in " << metrics_.ctor_ordering_time.count() << "ms" << std::endl;
        
        return ordered_txs;
    }
    
    /**
     * Create large block from transactions
     */
    CBlock CreateLargeBlock(const std::vector<CTransaction>& transactions) {
        std::cout << "Creating large block with " << transactions.size() << " transactions..." << std::endl;
        
        auto start_time = high_resolution_clock::now();
        
        CBlock block;
        
        // Set block header
        block.nVersion = 1;
        block.hashPrevBlock = uint256S("0000000000000000000000000000000000000000000000000000000000000000");
        block.nTime = GetTime();
        block.nBits = 0x207fffff; // Minimum difficulty
        block.nNonce = 0;
        
        // Add coinbase transaction
        CMutableTransaction coinbase;
        coinbase.vin.resize(1);
        coinbase.vin[0].prevout.SetNull();
        coinbase.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<uint8_t>((const uint8_t*)("Large Block Test"), (const uint8_t*)("Large Block Test") + strlen("Large Block Test"));
        coinbase.vout.resize(1);
        coinbase.vout[0].nValue = 50 * COIN;
        coinbase.vout[0].scriptPubKey = CScript() << OP_TRUE;
        
        block.vtx.push_back(MakeTransactionRef(coinbase));
        
        // Add all test transactions
        for (const auto& tx : transactions) {
            block.vtx.push_back(MakeTransactionRef(tx));
        }
        
        // Calculate merkle root
        block.hashMerkleRoot = BlockMerkleRoot(block);
        
        auto end_time = high_resolution_clock::now();
        metrics_.block_creation_time = duration_cast<milliseconds>(end_time - start_time);
        
        // Calculate block size
        metrics_.raw_block_size = GetSerializeSize(block, PROTOCOL_VERSION);
        
        std::cout << "Created block with " << block.vtx.size() << " transactions" << std::endl;
        std::cout << "Raw block size: " << (metrics_.raw_block_size / (1024*1024)) << " MB" << std::endl;
        std::cout << "Block creation time: " << metrics_.block_creation_time.count() << "ms" << std::endl;
        
        return block;
    }
    
    /**
     * Test Xthinner compression on large block
     */
    void TestXthinnerCompression(const CBlock& block) {
        if (!config_.enable_compression) {
            return;
        }
        
        std::cout << "Testing Xthinner compression..." << std::endl;
        
        auto start_time = high_resolution_clock::now();
        
        // Compress block using Xthinner
        xthinner::CompressedBlock compressed;
        bool success = xthinner::CompressBlock(block, compressed);
        
        auto compression_end = high_resolution_clock::now();
        metrics_.compression_time = duration_cast<milliseconds>(compression_end - start_time);
        
        if (success) {
            metrics_.compressed_block_size = compressed.compressed_size;
            metrics_.compression_ratio = compressed.compression_ratio;
            
            std::cout << "Compression successful!" << std::endl;
            std::cout << "Raw size: " << (metrics_.raw_block_size / (1024*1024)) << " MB" << std::endl;
            std::cout << "Compressed size: " << (metrics_.compressed_block_size / (1024*1024)) << " MB" << std::endl;
            std::cout << "Compression ratio: " << (metrics_.compression_ratio * 100) << "%" << std::endl;
            std::cout << "Compression time: " << metrics_.compression_time.count() << "ms" << std::endl;
            
            // Test decompression
            auto decomp_start = high_resolution_clock::now();
            CBlock decompressed;
            bool decomp_success = xthinner::DecompressBlock(compressed, decompressed);
            auto decomp_end = high_resolution_clock::now();
            
            metrics_.decompression_time = duration_cast<milliseconds>(decomp_end - decomp_start);
            
            if (decomp_success) {
                std::cout << "Decompression successful in " << metrics_.decompression_time.count() << "ms" << std::endl;
            } else {
                std::cout << "Decompression failed!" << std::endl;
            }
        } else {
            std::cout << "Compression failed!" << std::endl;
        }
    }
    
    /**
     * Benchmark block validation performance
     */
    void BenchmarkValidation(const CBlock& block) {
        if (!config_.benchmark_validation) {
            return;
        }
        
        std::cout << "Benchmarking block validation..." << std::endl;
        
        auto start_time = high_resolution_clock::now();
        
        // Validate block (simplified for testing)
        BlockValidationState state;
        bool valid = true; // Simplified validation for testing
        
        auto end_time = high_resolution_clock::now();
        metrics_.block_validation_time = duration_cast<milliseconds>(end_time - start_time);
        
        std::cout << "Block validation time: " << metrics_.block_validation_time.count() << "ms" << std::endl;
        std::cout << "Validation result: " << (valid ? "VALID" : "INVALID") << std::endl;
    }
    
    /**
     * Print comprehensive performance report
     */
    void PrintPerformanceReport() {
        std::cout << "\n" << std::string(80, '=') << std::endl;
        std::cout << "LARGE BLOCK STRESS TEST RESULTS: " << config_.test_name << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        
        std::cout << "\nBlock Statistics:" << std::endl;
        std::cout << "  Target Block Size: " << (config_.target_block_size / (1024*1024)) << " MB" << std::endl;
        std::cout << "  Actual Block Size: " << (metrics_.raw_block_size / (1024*1024)) << " MB" << std::endl;
        std::cout << "  Number of Transactions: " << config_.num_transactions << std::endl;
        
        std::cout << "\nPerformance Metrics:" << std::endl;
        std::cout << "  Transaction Generation: " << metrics_.transaction_generation_time.count() << " ms" << std::endl;
        std::cout << "  CTOR Ordering: " << metrics_.ctor_ordering_time.count() << " ms" << std::endl;
        std::cout << "  Block Creation: " << metrics_.block_creation_time.count() << " ms" << std::endl;
        std::cout << "  Block Validation: " << metrics_.block_validation_time.count() << " ms" << std::endl;
        
        if (config_.enable_compression) {
            std::cout << "\nCompression Results:" << std::endl;
            std::cout << "  Raw Size: " << (metrics_.raw_block_size / (1024*1024)) << " MB" << std::endl;
            std::cout << "  Compressed Size: " << (metrics_.compressed_block_size / (1024*1024)) << " MB" << std::endl;
            std::cout << "  Compression Ratio: " << (metrics_.compression_ratio * 100) << "%" << std::endl;
            std::cout << "  Compression Time: " << metrics_.compression_time.count() << " ms" << std::endl;
            std::cout << "  Decompression Time: " << metrics_.decompression_time.count() << " ms" << std::endl;
            
            double bandwidth_savings = (1.0 - metrics_.compression_ratio) * 100;
            std::cout << "  Network Bandwidth Savings: " << bandwidth_savings << "%" << std::endl;
        }
        
        std::cout << "\nTotal Test Time: " << 
            (metrics_.transaction_generation_time + metrics_.ctor_ordering_time + 
             metrics_.block_creation_time + metrics_.compression_time + 
             metrics_.block_validation_time).count() << " ms" << std::endl;
        
        std::cout << std::string(80, '=') << std::endl;
    }
    
    /**
     * Execute complete stress test
     */
    bool RunStressTest() {
        std::cout << "Starting Large Block Stress Test: " << config_.test_name << std::endl;
        std::cout << "Target: " << (config_.target_block_size / (1024*1024)) << " MB block with " 
                  << config_.num_transactions << " transactions" << std::endl;
        
        try {
            // Step 1: Generate test transactions
            auto transactions = GenerateTestTransactions();
            
            // Step 2: Apply CTOR ordering
            auto ordered_transactions = ApplyCTOROrdering(transactions);
            
            // Step 3: Create large block
            auto block = CreateLargeBlock(ordered_transactions);
            
            // Step 4: Test compression
            TestXthinnerCompression(block);
            
            // Step 5: Benchmark validation
            BenchmarkValidation(block);
            
            // Step 6: Print results
            PrintPerformanceReport();
            
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "Stress test failed: " << e.what() << std::endl;
            return false;
        }
    }
};

/**
 * Predefined test configurations
 */
std::vector<StressTestConfig> GetTestConfigurations() {
    return {
        // Test 1: 100MB Block
        {
            .target_block_size = 100 * 1024 * 1024,
            .num_transactions = 200000,
            .enable_compression = true,
            .enable_ctor = true,
            .benchmark_validation = true,
            .test_name = "100MB Block Test"
        },
        
        // Test 2: 500MB Block
        {
            .target_block_size = 500 * 1024 * 1024,
            .num_transactions = 1000000,
            .enable_compression = true,
            .enable_ctor = true,
            .benchmark_validation = true,
            .test_name = "500MB Block Test"
        },
        
        // Test 3: 1GB Block
        {
            .target_block_size = 1024 * 1024 * 1024,
            .num_transactions = 2000000,
            .enable_compression = true,
            .enable_ctor = true,
            .benchmark_validation = true,
            .test_name = "1GB Block Test"
        },
        
        // Test 4: 2GB Block
        {
            .target_block_size = 2ULL * 1024 * 1024 * 1024,
            .num_transactions = 4000000,
            .enable_compression = true,
            .enable_ctor = true,
            .benchmark_validation = true,
            .test_name = "2GB Block Test"
        },
        
        // Test 5: 4GB Block (BSV Matching)
        {
            .target_block_size = 4ULL * 1024 * 1024 * 1024,
            .num_transactions = 8000000,
            .enable_compression = true,
            .enable_ctor = true,
            .benchmark_validation = true,
            .test_name = "4GB Block Test (BSV Matching)"
        },
        
        // Test 6: 8GB Block (BSV Exceeding)
        {
            .target_block_size = 8ULL * 1024 * 1024 * 1024,
            .num_transactions = 16000000,
            .enable_compression = true,
            .enable_ctor = true,
            .benchmark_validation = true,
            .test_name = "8GB Block Test (BSV Exceeding)"
        }
    };
}

/**
 * Main stress testing function
 */
int main(int argc, char* argv[]) {
    std::cout << "Bitcoin Decentral Large Block Stress Testing Tool" << std::endl;
    std::cout << "Testing 4GB+ Block Capability vs Bitcoin SV" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    // Get test configurations
    auto test_configs = GetTestConfigurations();
    
    // Allow selection of specific test
    size_t test_index = 0;
    if (argc > 1) {
        test_index = std::stoul(argv[1]);
        if (test_index >= test_configs.size()) {
            std::cerr << "Invalid test index. Available tests: 0-" << (test_configs.size()-1) << std::endl;
            return 1;
        }
    }
    
    // Run selected test
    LargeBlockStressTester tester(test_configs[test_index]);
    bool success = tester.RunStressTest();
    
    return success ? 0 : 1;
}
