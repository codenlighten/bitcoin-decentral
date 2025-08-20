// Copyright (c) 2025 The Bitcoin Decentral developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOINDECENTRAL_SCALING_CTOR_ORDERING_H
#define BITCOINDECENTRAL_SCALING_CTOR_ORDERING_H

#include <primitives/transaction.h>
#include <primitives/block.h>
#include <vector>
#include <cstdint>

namespace ctor {

/**
 * Canonical Transaction Ordering (CTOR) for Bitcoin Decentral
 * 
 * CTOR eliminates transaction ordering bottlenecks by defining a canonical
 * ordering that allows for parallel processing and deterministic block
 * construction without requiring miners to coordinate transaction ordering.
 */

/**
 * Transaction ordering comparison function for CTOR
 * Orders transactions by:
 * 1. Transaction ID (txid) in lexicographic order
 * 2. Ensures deterministic, canonical ordering
 */
struct CTORComparator {
    bool operator()(const CTransactionRef& a, const CTransactionRef& b) const;
};

/**
 * Apply Canonical Transaction Ordering to a vector of transactions
 * @param transactions Vector of transactions to order
 * @return Vector of transactions in canonical order
 */
std::vector<CTransactionRef> ApplyCTOR(std::vector<CTransactionRef> transactions);

/**
 * Verify that transactions in a block follow CTOR
 * @param block Block to verify
 * @return true if transactions are in canonical order, false otherwise
 */
bool VerifyBlockCTOR(const CBlock& block);

/**
 * Check if CTOR is active for a given block height
 * @param height Block height to check
 * @return true if CTOR is active, false otherwise
 */
bool IsCTORActive(int height);

/**
 * Get the activation height for CTOR
 * @return Block height when CTOR becomes active
 */
int GetCTORActivationHeight();

/**
 * Sort transactions for mining using CTOR
 * This function is used by miners to order transactions in blocks
 * @param transactions Transactions to order for mining
 * @return Transactions in canonical order ready for block inclusion
 */
std::vector<CTransactionRef> SortTransactionsForMining(std::vector<CTransactionRef> transactions);

/**
 * Validate transaction ordering in mempool for CTOR compatibility
 * @param transactions Transactions to validate
 * @return true if ordering is CTOR-compatible, false otherwise
 */
bool ValidateMempoolCTOR(const std::vector<CTransactionRef>& transactions);

/**
 * CTOR activation parameters
 */
namespace activation {
    // CTOR activation height (to be determined by governance)
    static const int CTOR_ACTIVATION_HEIGHT = 1000;
    
    // CTOR version bit for BIP9 activation
    static const int CTOR_VERSION_BIT = 28;
    
    // CTOR deployment name
    extern const char* CTOR_DEPLOYMENT_NAME;
}

} // namespace ctor

#endif // BITCOINDECENTRAL_SCALING_CTOR_ORDERING_H
