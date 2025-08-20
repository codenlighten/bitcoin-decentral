// Copyright (c) 2025 The Bitcoin Decentral developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <scaling/ctor/validation.h>
#include <scaling/ctor/ordering.h>
#include <logging.h>
#include <util/strencodings.h>

namespace ctor {

bool ValidateBlockCTOR(const CBlock& block, const CBlockIndex* pindex, BlockValidationState& state) {
    if (!pindex) {
        SetCTORError(CTORError::CONSENSUS_FAILURE, state, "Invalid block index for CTOR validation");
        return false;
    }
    
    // Check if CTOR is active for this block height
    if (!IsCTORActive(pindex->nHeight)) {
        LogPrintf("CTOR: Not active at height %d, skipping validation\n", pindex->nHeight);
        return true;
    }
    
    LogPrintf("CTOR: Validating block at height %d\n", pindex->nHeight);
    
    // Validate transaction ordering
    if (!CheckBlockTransactionOrdering(block, pindex->nHeight)) {
        SetCTORError(CTORError::INVALID_ORDERING, state, 
                    strprintf("Block transactions not in canonical order at height %d", pindex->nHeight));
        return false;
    }
    
    LogPrintf("CTOR: Block validation successful at height %d\n", pindex->nHeight);
    return true;
}

bool CheckBlockTransactionOrdering(const CBlock& block, int height) {
    if (!IsCTORActive(height)) {
        return true; // CTOR not active, ordering not enforced
    }
    
    // Skip coinbase transaction (index 0)
    for (size_t i = 2; i < block.vtx.size(); ++i) {
        const uint256& prev_hash = block.vtx[i-1]->GetHash();
        const uint256& curr_hash = block.vtx[i]->GetHash();
        
        // Check canonical ordering (lexicographic by txid)
        if (!(prev_hash < curr_hash)) {
            LogPrintf("CTOR: Invalid transaction ordering at position %d\n", (int)i);
            LogPrintf("CTOR: Previous txid: %s\n", prev_hash.ToString().c_str());
            LogPrintf("CTOR: Current txid: %s\n", curr_hash.ToString().c_str());
            return false;
        }
    }
    
    return true;
}

bool ValidateBlockTemplateCTOR(const CBlock& block_template, int height) {
    if (!IsCTORActive(height)) {
        return true; // CTOR not active, no validation needed
    }
    
    LogPrintf("CTOR: Validating block template for height %d\n", height);
    
    // Validate that block template follows CTOR
    return CheckBlockTransactionOrdering(block_template, height);
}

bool CTORConsensusValidation(const CBlock& block, const CBlockIndex* pindex, 
                            const Consensus::Params& params, BlockValidationState& state) {
    // Perform comprehensive CTOR consensus validation
    if (!ValidateBlockCTOR(block, pindex, state)) {
        return false;
    }
    
    // Additional consensus checks can be added here
    // For example: activation signaling validation, fork protection, etc.
    
    return true;
}

bool IsCTORActiveForBlock(const CBlockIndex* pindex, const Consensus::Params& params) {
    if (!pindex) {
        return false;
    }
    
    // For now, use simple height-based activation
    // TODO: Implement proper BIP9 activation logic with consensus params
    return IsCTORActive(pindex->nHeight);
}

CTORStatus GetCTORStatus(const CBlockIndex* chain_tip, const Consensus::Params& params) {
    if (!chain_tip) {
        return CTORStatus::INACTIVE;
    }
    
    if (IsCTORActiveForBlock(chain_tip, params)) {
        return CTORStatus::ACTIVE;
    }
    
    // Check if we're in activation period
    int activation_height = GetCTORActivationHeight();
    if (chain_tip->nHeight >= activation_height - 100 && 
        chain_tip->nHeight < activation_height) {
        // In activation window but not yet active
        return CTORStatus::INACTIVE;
    }
    
    return CTORStatus::INACTIVE;
}

void SetCTORError(CTORError error, BlockValidationState& state, const std::string& debug_message) {
    switch (error) {
        case CTORError::INVALID_ORDERING:
            state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "ctor-invalid-ordering", debug_message);
            break;
        case CTORError::ACTIVATION_VIOLATION:
            state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "ctor-activation-violation", debug_message);
            break;
        case CTORError::CONSENSUS_FAILURE:
            state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "ctor-consensus-failure", debug_message);
            break;
    }
    
    LogPrintf("CTOR Error: %s\n", debug_message.c_str());
}

} // namespace ctor
