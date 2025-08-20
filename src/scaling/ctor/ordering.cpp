// Copyright (c) 2025 The Bitcoin Decentral developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <scaling/ctor/ordering.h>
#include <algorithm>
#include <logging.h>
#include <util/strencodings.h>

namespace ctor {

// Define extern variables from header
namespace activation {
    const char* CTOR_DEPLOYMENT_NAME = "ctor";
}

bool CTORComparator::operator()(const CTransactionRef& a, const CTransactionRef& b) const {
    // Order transactions by transaction ID (txid) in lexicographic order
    // This ensures deterministic, canonical ordering
    return a->GetHash() < b->GetHash();
}

std::vector<CTransactionRef> ApplyCTOR(std::vector<CTransactionRef> transactions) {
    LogPrintf("CTOR: Applying canonical transaction ordering to %d transactions\n", (int)transactions.size());
    
    // Sort transactions using CTOR comparator
    std::sort(transactions.begin(), transactions.end(), CTORComparator());
    
    LogPrintf("CTOR: Canonical ordering applied successfully\n");
    return transactions;
}

bool VerifyBlockCTOR(const CBlock& block) {
    if (!IsCTORActive(0)) { // TODO: Pass actual block height
        return true; // CTOR not active, skip verification
    }
    
    LogPrintf("CTOR: Verifying block transaction ordering\n");
    
    // Extract transactions from block (skip coinbase)
    std::vector<CTransactionRef> transactions;
    for (size_t i = 1; i < block.vtx.size(); ++i) {
        transactions.push_back(block.vtx[i]);
    }
    
    // Check if transactions are in canonical order
    for (size_t i = 1; i < transactions.size(); ++i) {
        if (!(transactions[i-1]->GetHash() < transactions[i]->GetHash())) {
            LogPrintf("CTOR: Block transaction ordering violation at position %d\n", (int)i);
            return false;
        }
    }
    
    LogPrintf("CTOR: Block transaction ordering verified successfully\n");
    return true;
}

bool IsCTORActive(int height) {
    // For now, CTOR is active after activation height
    // TODO: Implement proper BIP9 activation logic
    return height >= activation::CTOR_ACTIVATION_HEIGHT;
}

int GetCTORActivationHeight() {
    return activation::CTOR_ACTIVATION_HEIGHT;
}

std::vector<CTransactionRef> SortTransactionsForMining(std::vector<CTransactionRef> transactions) {
    LogPrintf("CTOR: Sorting %d transactions for mining\n", (int)transactions.size());
    
    // Apply CTOR ordering for mining
    return ApplyCTOR(std::move(transactions));
}

bool ValidateMempoolCTOR(const std::vector<CTransactionRef>& transactions) {
    if (!IsCTORActive(0)) { // TODO: Pass actual chain tip height
        return true; // CTOR not active, no validation needed
    }
    
    LogPrintf("CTOR: Validating mempool transaction ordering\n");
    
    // Check if transactions would be in canonical order
    for (size_t i = 1; i < transactions.size(); ++i) {
        if (!(transactions[i-1]->GetHash() < transactions[i]->GetHash())) {
            LogPrintf("CTOR: Mempool ordering violation detected\n");
            return false;
        }
    }
    
    LogPrintf("CTOR: Mempool transaction ordering validated\n");
    return true;
}

} // namespace ctor
