#ifndef BITCOIN_CRYPTO_QUANTUM_H
#define BITCOIN_CRYPTO_QUANTUM_H

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

class uint256;

/**
 * Bitcoin Decentral Quantum-Resistant Cryptography System
 * 
 * This module implements post-quantum cryptographic algorithms to protect
 * Bitcoin Decentral against future quantum computing threats. It provides
 * a hybrid security model that gradually transitions from classical to
 * quantum-resistant cryptography while maintaining backward compatibility.
 * 
 * Key Features:
 * - NIST-approved post-quantum cryptographic algorithms
 * - Hybrid classical/quantum-resistant security model
 * - Quantum-safe digital signatures and key exchange
 * - Future-proof hash functions and encryption
 * - Gradual migration path from classical cryptography
 * - Cross-chain quantum-resistant operations
 */

namespace crypto {
namespace quantum {

/**
 * Quantum-resistant algorithm types
 */
enum QuantumAlgorithmType {
    // Digital Signature Algorithms
    QR_DILITHIUM2,          // CRYSTALS-Dilithium (NIST Level 2)
    QR_DILITHIUM3,          // CRYSTALS-Dilithium (NIST Level 3)
    QR_DILITHIUM5,          // CRYSTALS-Dilithium (NIST Level 5)
    QR_FALCON512,           // FALCON (NIST Level 1)
    QR_FALCON1024,          // FALCON (NIST Level 5)
    QR_SPHINCS_SHA256,      // SPHINCS+ with SHA-256
    QR_SPHINCS_SHAKE256,    // SPHINCS+ with SHAKE-256
    
    // Key Encapsulation Mechanisms
    QR_KYBER512,            // CRYSTALS-Kyber (NIST Level 1)
    QR_KYBER768,            // CRYSTALS-Kyber (NIST Level 3)
    QR_KYBER1024,           // CRYSTALS-Kyber (NIST Level 5)
    
    // Hash Functions
    QR_SHA3_256,            // SHA-3 256-bit
    QR_SHA3_512,            // SHA-3 512-bit
    QR_SHAKE128,            // SHAKE-128
    QR_SHAKE256,            // SHAKE-256
    
    // Hybrid Algorithms
    QR_HYBRID_ECDSA_DILITHIUM,  // ECDSA + Dilithium hybrid
    QR_HYBRID_RSA_FALCON,       // RSA + FALCON hybrid
    QR_HYBRID_ECDH_KYBER        // ECDH + Kyber hybrid
};

/**
 * Quantum security levels
 */
enum QuantumSecurityLevel {
    QS_LEVEL_1 = 1,         // Equivalent to AES-128
    QS_LEVEL_2 = 2,         // Equivalent to AES-128 (conservative)
    QS_LEVEL_3 = 3,         // Equivalent to AES-192
    QS_LEVEL_4 = 4,         // Equivalent to AES-192 (conservative)
    QS_LEVEL_5 = 5          // Equivalent to AES-256
};

/**
 * Quantum-resistant key pair
 */
struct QuantumKeyPair {
    QuantumAlgorithmType algorithm;     // Algorithm type
    QuantumSecurityLevel security_level; // Security level
    std::vector<uint8_t> public_key;    // Public key data
    std::vector<uint8_t> private_key;   // Private key data
    uint32_t key_size_public;           // Public key size in bytes
    uint32_t key_size_private;          // Private key size in bytes
    uint64_t generation_time;           // Key generation timestamp
    bool is_hybrid;                     // Whether this is a hybrid key
    
    QuantumKeyPair() : algorithm(QR_DILITHIUM3), security_level(QS_LEVEL_3),
                      key_size_public(0), key_size_private(0), generation_time(0),
                      is_hybrid(false) {}
};

/**
 * Quantum-resistant signature
 */
struct QuantumSignature {
    QuantumAlgorithmType algorithm;     // Signature algorithm
    std::vector<uint8_t> signature;     // Signature data
    std::vector<uint8_t> public_key;    // Public key for verification
    uint32_t signature_size;            // Signature size in bytes
    uint64_t creation_time;             // Signature creation timestamp
    bool is_hybrid;                     // Whether this is a hybrid signature
    
    QuantumSignature() : algorithm(QR_DILITHIUM3), signature_size(0),
                        creation_time(0), is_hybrid(false) {}
};

/**
 * Quantum threat assessment
 */
struct QuantumThreatLevel {
    enum ThreatLevel {
        THREAT_MINIMAL,     // No immediate quantum threat
        THREAT_LOW,         // Quantum computers exist but not threatening
        THREAT_MODERATE,    // Quantum computers pose potential threat
        THREAT_HIGH,        // Quantum computers pose significant threat
        THREAT_CRITICAL     // Quantum computers can break current crypto
    };
    
    ThreatLevel current_level;          // Current threat assessment
    uint64_t assessment_time;           // Time of last assessment
    uint32_t quantum_computer_qubits;   // Estimated quantum computer capability
    double crypto_break_probability;    // Probability of crypto being broken
    uint32_t estimated_years_to_break;  // Estimated years until crypto is broken
    
    QuantumThreatLevel() : current_level(THREAT_MINIMAL), assessment_time(0),
                          quantum_computer_qubits(0), crypto_break_probability(0.0),
                          estimated_years_to_break(100) {}
};

/**
 * Quantum migration state
 */
struct QuantumMigrationState {
    enum MigrationPhase {
        PHASE_CLASSICAL,        // Pure classical cryptography
        PHASE_HYBRID_OPTIONAL,  // Hybrid crypto available but optional
        PHASE_HYBRID_REQUIRED,  // Hybrid crypto required for new operations
        PHASE_QUANTUM_ONLY      // Pure quantum-resistant cryptography
    };
    
    MigrationPhase current_phase;       // Current migration phase
    uint32_t migration_height;          // Block height of migration activation
    double classical_usage_percentage;  // Percentage still using classical crypto
    double hybrid_usage_percentage;     // Percentage using hybrid crypto
    double quantum_usage_percentage;    // Percentage using quantum-resistant crypto
    uint32_t migration_deadline_height; // Deadline for full migration
    
    QuantumMigrationState() : current_phase(PHASE_CLASSICAL), migration_height(0),
                             classical_usage_percentage(100.0), hybrid_usage_percentage(0.0),
                             quantum_usage_percentage(0.0), migration_deadline_height(0) {}
};

/**
 * Core quantum-resistant functions
 */

/**
 * Initialize quantum-resistant cryptography system
 */
bool InitializeQuantumCryptography();

/**
 * Generate quantum-resistant key pair
 */
bool GenerateQuantumKeyPair(QuantumAlgorithmType algorithm, QuantumSecurityLevel level,
                           QuantumKeyPair& key_pair);

/**
 * Create quantum-resistant signature
 */
bool CreateQuantumSignature(const std::vector<uint8_t>& message,
                           const QuantumKeyPair& key_pair,
                           QuantumSignature& signature);

/**
 * Verify quantum-resistant signature
 */
bool VerifyQuantumSignature(const std::vector<uint8_t>& message,
                           const QuantumSignature& signature);

/**
 * Generate hybrid signature (classical + quantum-resistant)
 */
bool CreateHybridSignature(const std::vector<uint8_t>& message,
                          const QuantumKeyPair& classical_key,
                          const QuantumKeyPair& quantum_key,
                          QuantumSignature& hybrid_signature);

/**
 * Verify hybrid signature
 */
bool VerifyHybridSignature(const std::vector<uint8_t>& message,
                          const QuantumSignature& hybrid_signature);

/**
 * Key encapsulation and exchange
 */

/**
 * Generate shared secret using quantum-resistant KEM
 */
bool GenerateSharedSecret(const QuantumKeyPair& local_key,
                         const std::vector<uint8_t>& remote_public_key,
                         std::vector<uint8_t>& shared_secret);

/**
 * Encapsulate secret for quantum-resistant key exchange
 */
bool EncapsulateSecret(const std::vector<uint8_t>& public_key,
                      std::vector<uint8_t>& ciphertext,
                      std::vector<uint8_t>& shared_secret);

/**
 * Decapsulate secret from quantum-resistant key exchange
 */
bool DecapsulateSecret(const std::vector<uint8_t>& ciphertext,
                      const QuantumKeyPair& key_pair,
                      std::vector<uint8_t>& shared_secret);

/**
 * Hash functions
 */

/**
 * Compute quantum-resistant hash
 */
uint256 ComputeQuantumHash(const std::vector<uint8_t>& data,
                          QuantumAlgorithmType hash_algorithm);

/**
 * Compute SHAKE hash with variable output length
 */
std::vector<uint8_t> ComputeShakeHash(const std::vector<uint8_t>& data,
                                     uint32_t output_length,
                                     QuantumAlgorithmType shake_variant);

/**
 * Verify hash integrity
 */
bool VerifyQuantumHash(const std::vector<uint8_t>& data,
                      const uint256& expected_hash,
                      QuantumAlgorithmType hash_algorithm);

/**
 * Migration and compatibility functions
 */

/**
 * Check if quantum migration is required
 */
bool IsQuantumMigrationRequired(uint32_t current_height);

/**
 * Get current migration phase
 */
QuantumMigrationState::MigrationPhase GetCurrentMigrationPhase();

/**
 * Initiate quantum migration
 */
bool InitiateQuantumMigration(uint32_t activation_height, uint32_t deadline_height);

/**
 * Convert classical key to hybrid key
 */
bool ConvertToHybridKey(const std::vector<uint8_t>& classical_key,
                       QuantumAlgorithmType quantum_algorithm,
                       QuantumKeyPair& hybrid_key);

/**
 * Validate quantum-resistant address
 */
bool ValidateQuantumAddress(const std::string& address);

/**
 * Generate quantum-resistant address
 */
std::string GenerateQuantumAddress(const QuantumKeyPair& key_pair);

/**
 * Threat assessment and monitoring
 */

/**
 * Assess current quantum threat level
 */
QuantumThreatLevel AssessQuantumThreat();

/**
 * Update quantum threat assessment
 */
bool UpdateQuantumThreatAssessment(uint32_t quantum_qubits,
                                  double break_probability,
                                  uint32_t years_to_break);

/**
 * Monitor quantum computing developments
 */
void MonitorQuantumDevelopments();

/**
 * Check if immediate migration is needed
 */
bool IsImmediateMigrationNeeded();

/**
 * Algorithm management
 */

/**
 * Get recommended quantum algorithm
 */
QuantumAlgorithmType GetRecommendedAlgorithm(QuantumSecurityLevel level);

/**
 * Check algorithm security level
 */
QuantumSecurityLevel GetAlgorithmSecurityLevel(QuantumAlgorithmType algorithm);

/**
 * Validate algorithm parameters
 */
bool ValidateAlgorithmParameters(QuantumAlgorithmType algorithm,
                                QuantumSecurityLevel level);

/**
 * Get algorithm key sizes
 */
void GetAlgorithmKeySizes(QuantumAlgorithmType algorithm,
                         uint32_t& public_key_size,
                         uint32_t& private_key_size,
                         uint32_t& signature_size);

/**
 * Cross-chain quantum security
 */

/**
 * Generate quantum-resistant bridge signature
 */
bool CreateQuantumBridgeSignature(const std::vector<uint8_t>& bridge_data,
                                 const QuantumKeyPair& validator_key,
                                 QuantumSignature& bridge_signature);

/**
 * Verify cross-chain quantum signature
 */
bool VerifyCrossChainQuantumSignature(const std::vector<uint8_t>& chain_data,
                                     const QuantumSignature& signature,
                                     const std::string& source_chain);

/**
 * Generate quantum-resistant IBC proof
 */
std::vector<uint8_t> GenerateQuantumIBCProof(const std::vector<uint8_t>& commitment,
                                            uint64_t height,
                                            const QuantumKeyPair& validator_key);

/**
 * Performance and optimization
 */

/**
 * Optimize quantum operations for performance
 */
void OptimizeQuantumPerformance();

/**
 * Benchmark quantum algorithms
 */
struct QuantumBenchmark {
    QuantumAlgorithmType algorithm;
    uint64_t key_generation_time_ms;
    uint64_t signature_time_ms;
    uint64_t verification_time_ms;
    uint32_t memory_usage_kb;
    
    QuantumBenchmark() : algorithm(QR_DILITHIUM3), key_generation_time_ms(0),
                        signature_time_ms(0), verification_time_ms(0), memory_usage_kb(0) {}
};

QuantumBenchmark BenchmarkQuantumAlgorithm(QuantumAlgorithmType algorithm);

/**
 * Get quantum performance statistics
 */
struct QuantumStats {
    uint32_t total_quantum_keys_generated;
    uint32_t total_quantum_signatures_created;
    uint32_t total_quantum_signatures_verified;
    uint32_t total_hybrid_operations;
    double average_signature_time_ms;
    double average_verification_time_ms;
    uint32_t quantum_migration_percentage;
    
    QuantumStats() : total_quantum_keys_generated(0), total_quantum_signatures_created(0),
                    total_quantum_signatures_verified(0), total_hybrid_operations(0),
                    average_signature_time_ms(0.0), average_verification_time_ms(0.0),
                    quantum_migration_percentage(0) {}
};

QuantumStats GetQuantumStatistics();

/**
 * Utility functions
 */

/**
 * Get quantum migration state
 */
QuantumMigrationState GetQuantumMigrationState();

/**
 * Update quantum migration state
 */
bool UpdateQuantumMigrationState(const QuantumMigrationState& state);

/**
 * Get algorithm name
 */
std::string GetAlgorithmName(QuantumAlgorithmType algorithm);

/**
 * Parse algorithm from string
 */
QuantumAlgorithmType ParseAlgorithmName(const std::string& name);

/**
 * Check quantum readiness
 */
bool IsQuantumReady();

/**
 * Perform quantum maintenance
 */
void PerformQuantumMaintenance();

} // namespace quantum
} // namespace crypto

#endif // BITCOIN_CRYPTO_QUANTUM_H
