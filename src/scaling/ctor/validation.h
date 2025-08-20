// Copyright (c) 2025 The Bitcoin Decentral developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOINDECENTRAL_SCALING_CTOR_VALIDATION_H
#define BITCOINDECENTRAL_SCALING_CTOR_VALIDATION_H

#include <consensus/validation.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <chain.h>

namespace ctor {

/**
 * CTOR Validation for Bitcoin Decentral
 * 
 * This module provides validation functions for Canonical Transaction Ordering
 * that integrate with Bitcoin Decentral's consensus validation system.
 */

/**
 * Validate CTOR compliance for a block
 * @param block Block to validate
 * @param pindex Block index for height information
 * @param state Validation state for error reporting
 * @return true if block passes CTOR validation, false otherwise
 */
bool ValidateBlockCTOR(const CBlock& block, const CBlockIndex* pindex, BlockValidationState& state);

/**
 * Check if a block's transaction ordering follows CTOR rules
 * @param block Block to check
 * @param height Block height for activation check
 * @return true if ordering is valid, false otherwise
 */
bool CheckBlockTransactionOrdering(const CBlock& block, int height);

/**
 * Validate CTOR for block template creation (mining)
 * @param block_template Block template to validate
 * @param height Target block height
 * @return true if template is CTOR-compliant, false otherwise
 */
bool ValidateBlockTemplateCTOR(const CBlock& block_template, int height);

/**
 * CTOR consensus rule validation
 * @param block Block to validate
 * @param pindex Block index
 * @param params Consensus parameters
 * @param state Validation state
 * @return true if consensus rules are satisfied, false otherwise
 */
bool CTORConsensusValidation(const CBlock& block, const CBlockIndex* pindex, 
                            const Consensus::Params& params, BlockValidationState& state);

/**
 * Check if CTOR activation conditions are met
 * @param pindex Block index to check
 * @param params Consensus parameters
 * @return true if CTOR should be active, false otherwise
 */
bool IsCTORActiveForBlock(const CBlockIndex* pindex, const Consensus::Params& params);

/**
 * Get CTOR activation status for chain tip
 * @param chain_tip Current chain tip
 * @param params Consensus parameters
 * @return CTOR activation status
 */
enum class CTORStatus {
    INACTIVE,    // CTOR not yet active
    ACTIVE,      // CTOR is active and enforced
    FAILED       // CTOR activation failed
};

CTORStatus GetCTORStatus(const CBlockIndex* chain_tip, const Consensus::Params& params);

/**
 * CTOR error codes for validation failures
 */
enum class CTORError {
    INVALID_ORDERING,        // Transactions not in canonical order
    ACTIVATION_VIOLATION,    // CTOR rules violated during activation
    CONSENSUS_FAILURE       // General consensus rule failure
};

/**
 * Convert CTOR error to validation state
 * @param error CTOR error code
 * @param state Validation state to update
 * @param debug_message Debug message for logging
 */
void SetCTORError(CTORError error, BlockValidationState& state, const std::string& debug_message);

} // namespace ctor

#endif // BITCOINDECENTRAL_SCALING_CTOR_VALIDATION_H
