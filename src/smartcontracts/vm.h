#ifndef BITCOIN_SMARTCONTRACTS_VM_H
#define BITCOIN_SMARTCONTRACTS_VM_H

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <map>

class CTransaction;
class CBlock;
#include <uint256.h>
namespace Consensus { struct Params; }

/**
 * Bitcoin Decentral Smart Contract Virtual Machine
 * 
 * This module implements advanced smart contract capabilities for Bitcoin Decentral,
 * building upon the revolutionary scaling foundation (CTOR, Unbounded Blocks, 
 * Xthinner, Advanced Mempool) to create the world's most complete blockchain platform.
 * 
 * Key Features:
 * - Enhanced Bitcoin Script with advanced opcodes
 * - EVM-compatible virtual machine layer
 * - Seamless smart contract deployment and execution
 * - Gas optimization leveraging scaling benefits
 * - Developer-friendly SDK and tooling
 * - Enterprise-grade contract execution
 */

namespace smartcontracts {

/**
 * Smart contract constants
 */
namespace vm {
    // Virtual machine types
    static const int VM_TYPE_BITCOIN_SCRIPT = 1;
    static const int VM_TYPE_EVM_COMPATIBLE = 2;
    
    // Gas limits and pricing
    static const uint64_t MAX_GAS_LIMIT = 30000000;  // 30M gas per contract
    static const uint64_t BASE_GAS_PRICE = 1000;     // Base gas price in satoshis
    
    // Contract size limits
    static const size_t MAX_CONTRACT_SIZE = 24576;   // 24KB max contract size
    static const size_t MAX_STORAGE_SIZE = 1048576;  // 1MB max storage per contract
    
    // Execution limits
    static const int MAX_CALL_DEPTH = 1024;         // Maximum call stack depth
    static const int MAX_EXECUTION_TIME_MS = 5000;  // 5 second execution limit
    
    // Smart contract activation height
    static const int SMART_CONTRACT_ACTIVATION_HEIGHT = 5000;
}

/**
 * Contract execution result
 */
enum ContractResult {
    CONTRACT_SUCCESS = 0,
    CONTRACT_OUT_OF_GAS = 1,
    CONTRACT_EXECUTION_ERROR = 2,
    CONTRACT_INVALID_OPCODE = 3,
    CONTRACT_STACK_OVERFLOW = 4,
    CONTRACT_TIMEOUT = 5,
    CONTRACT_STORAGE_LIMIT = 6
};

/**
 * Smart contract state
 */
struct ContractState {
    uint256 contract_address;           // Contract address
    std::vector<uint8_t> code;         // Contract bytecode
    std::map<uint256, uint256> storage; // Contract storage
    uint64_t balance;                   // Contract balance
    uint64_t nonce;                     // Contract nonce
    int vm_type;                        // Virtual machine type
    bool is_active;                     // Contract active status
    
    ContractState() : balance(0), nonce(0), vm_type(vm::VM_TYPE_BITCOIN_SCRIPT), is_active(true) {}
};

/**
 * Contract execution context
 */
struct ExecutionContext {
    uint256 tx_hash;                    // Transaction hash
    uint256 block_hash;                 // Block hash
    uint32_t block_height;              // Block height
    uint64_t block_timestamp;           // Block timestamp
    uint256 caller_address;             // Caller address
    uint256 contract_address;           // Contract address
    uint64_t gas_limit;                 // Gas limit
    uint64_t gas_price;                 // Gas price
    uint64_t value;                     // Value sent
    std::vector<uint8_t> input_data;    // Input data
    
    ExecutionContext() : block_height(0), block_timestamp(0), gas_limit(0), gas_price(0), value(0) {}
};

/**
 * Contract execution result
 */
struct ExecutionResult {
    ContractResult result_code;         // Execution result
    std::vector<uint8_t> return_data;   // Return data
    uint64_t gas_used;                  // Gas consumed
    std::vector<uint256> logs;          // Event logs
    std::string error_message;          // Error message if failed
    
    ExecutionResult() : result_code(CONTRACT_SUCCESS), gas_used(0) {}
};

/**
 * Enhanced Bitcoin Script opcodes
 */
enum EnhancedOpcodes {
    // Arithmetic operations
    OP_ADDMOD = 0xf0,                   // (a b n -- (a+b)%n)
    OP_MULMOD = 0xf1,                   // (a b n -- (a*b)%n)
    OP_EXP = 0xf2,                      // (a b -- a^b)
    
    // Cryptographic operations
    OP_KECCAK256 = 0xf3,                // (data -- hash)
    OP_BLAKE2B = 0xf4,                  // (data -- hash)
    OP_ECRECOVER = 0xf5,                // (hash v r s -- address)
    
    // Storage operations
    OP_SLOAD = 0xf6,                    // (key -- value)
    OP_SSTORE = 0xf7,                   // (key value --)
    
    // Contract operations
    OP_CREATE = 0xf8,                   // (value code -- address)
    OP_CALL = 0xf9,                     // (gas address value data -- success)
    OP_DELEGATECALL = 0xfa,             // (gas address data -- success)
    
    // System operations
    OP_BLOCKHASH = 0xfb,                // (number -- hash)
    OP_COINBASE = 0xfc,                 // (-- address)
    OP_TIMESTAMP = 0xfd,                // (-- timestamp)
    OP_GASLIMIT = 0xfe,                 // (-- gaslimit)
    OP_DIFFICULTY = 0xff                // (-- difficulty)
};

/**
 * Core smart contract functions
 */

/**
 * Initialize smart contract virtual machine
 */
bool InitializeSmartContractVM(const Consensus::Params& params);

/**
 * Deploy new smart contract
 */
uint256 DeployContract(const std::vector<uint8_t>& bytecode, 
                      const ExecutionContext& context,
                      ExecutionResult& result);

/**
 * Execute smart contract function
 */
ExecutionResult ExecuteContract(const uint256& contract_address,
                               const ExecutionContext& context);

/**
 * Get contract state
 */
ContractState GetContractState(const uint256& contract_address);

/**
 * Update contract state
 */
bool UpdateContractState(const uint256& contract_address, const ContractState& state);

/**
 * Validate contract bytecode
 */
bool ValidateContractBytecode(const std::vector<uint8_t>& bytecode, int vm_type);

/**
 * Calculate gas cost for operation
 */
uint64_t CalculateGasCost(uint8_t opcode, const std::vector<uint8_t>& data);

/**
 * Execute enhanced Bitcoin Script
 */
ExecutionResult ExecuteEnhancedScript(const std::vector<uint8_t>& script,
                                     const ExecutionContext& context);

/**
 * Execute EVM-compatible bytecode
 */
ExecutionResult ExecuteEVMBytecode(const std::vector<uint8_t>& bytecode,
                                  const ExecutionContext& context);

/**
 * Handle contract storage operations
 */
uint256 LoadContractStorage(const uint256& contract_address, const uint256& key);
bool StoreContractStorage(const uint256& contract_address, const uint256& key, const uint256& value);

/**
 * Process contract events and logs
 */
void EmitContractEvent(const uint256& contract_address, const std::vector<uint256>& topics,
                      const std::vector<uint8_t>& data);

/**
 * Validate contract execution limits
 */
bool ValidateExecutionLimits(const ExecutionContext& context);

/**
 * Handle contract creation
 */
uint256 CreateContract(const std::vector<uint8_t>& init_code,
                      const ExecutionContext& context,
                      ExecutionResult& result);

/**
 * Handle contract calls
 */
ExecutionResult CallContract(const uint256& target_address,
                           const ExecutionContext& context);

/**
 * Manage contract balance and transfers
 */
bool TransferContractBalance(const uint256& from_address, const uint256& to_address, uint64_t amount);

/**
 * Get contract balance
 */
uint64_t GetContractBalance(const uint256& contract_address);

/**
 * Check if smart contracts are active
 */
bool IsSmartContractActive(int height, const Consensus::Params& params);

/**
 * Get smart contract activation height
 */
int GetSmartContractActivationHeight(const Consensus::Params& params);

/**
 * Integrate with CTOR for deterministic execution
 */
std::vector<ExecutionResult> ExecuteContractsInCTOROrder(
    const std::vector<CTransaction>& transactions,
    const ExecutionContext& block_context);

/**
 * Optimize gas usage with scaling features
 */
uint64_t OptimizeGasWithScaling(uint64_t base_gas, const ExecutionContext& context);

/**
 * Handle contract state compression for Xthinner
 */
std::vector<uint8_t> CompressContractState(const ContractState& state);
ContractState DecompressContractState(const std::vector<uint8_t>& compressed_data);

/**
 * Manage contract storage with advanced mempool
 */
void OptimizeContractStorageForMempool(const uint256& contract_address);

/**
 * Validate contract transaction in context of scaling features
 */
bool ValidateContractTransaction(const CTransaction& tx, const Consensus::Params& params);

/**
 * Get contract execution statistics
 */
struct ContractStats {
    uint64_t total_contracts;
    uint64_t total_executions;
    uint64_t total_gas_used;
    uint64_t average_execution_time;
    double success_rate;
};

ContractStats GetContractExecutionStats();

/**
 * Monitor contract performance
 */
void MonitorContractPerformance();

/**
 * Cleanup expired contract data
 */
void CleanupExpiredContractData();

} // namespace smartcontracts

#endif // BITCOIN_SMARTCONTRACTS_VM_H
