// Copyright (c) 2025 The Bitcoin Decentral developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOINDECENTRAL_SCALING_CTOR_CONSENSUS_H
#define BITCOINDECENTRAL_SCALING_CTOR_CONSENSUS_H

#include <consensus/params.h>
#include <versionbits.h>
#include <primitives/block.h>
#include <chain.h>

namespace ctor {

/**
 * CTOR Consensus Integration for Bitcoin Decentral
 * 
 * This module handles the consensus-level integration of Canonical Transaction
 * Ordering, including activation logic, fork management, and consensus rules.
 */

/**
 * CTOR deployment parameters for BIP9 activation
 */
struct CTORDeployment {
    /** The bit position for CTOR signaling */
    int bit;
    
    /** Start time for CTOR activation signaling */
    int64_t nStartTime;
    
    /** Timeout for CTOR activation */
    int64_t nTimeout;
    
    /** Minimum activation threshold (percentage of blocks) */
    int nThreshold;
    
    /** Window size for activation signaling */
    int nPeriod;
};

/**
 * Get CTOR deployment parameters
 * @return CTOR deployment configuration
 */
CTORDeployment GetCTORDeployment();

/**
 * Check CTOR activation state using BIP9
 * @param pindex Block index to check
 * @param params Consensus parameters
 * @return BIP9 state for CTOR
 */
ThresholdState GetCTORState(const CBlockIndex* pindex, const Consensus::Params& params);

/**
 * Initialize CTOR consensus parameters
 * @param params Consensus parameters to update
 */
void InitializeCTORConsensus(Consensus::Params& params);

/**
 * Check if block signals CTOR activation
 * @param block Block to check
 * @return true if block signals CTOR support
 */
bool BlockSignalsCTOR(const CBlock& block);

/**
 * Validate CTOR activation signaling in block
 * @param block Block to validate
 * @param pindex Block index
 * @param params Consensus parameters
 * @return true if signaling is valid
 */
bool ValidateCTORSignaling(const CBlock& block, const CBlockIndex* pindex, 
                          const Consensus::Params& params);

/**
 * Get the block height when CTOR becomes active
 * @param pindex Current block index
 * @param params Consensus parameters
 * @return Activation height, or -1 if not yet determined
 */
int GetCTORActivationHeight(const CBlockIndex* pindex, const Consensus::Params& params);

/**
 * Check if we're in CTOR grace period (allowing both old and new rules)
 * @param height Block height to check
 * @param params Consensus parameters
 * @return true if in grace period
 */
bool IsInCTORGracePeriod(int height, const Consensus::Params& params);

/**
 * CTOR fork protection - ensure we don't accept invalid chains
 * @param pindex Block index to validate
 * @param params Consensus parameters
 * @return true if chain is valid under CTOR rules
 */
bool ValidateCTORChain(const CBlockIndex* pindex, const Consensus::Params& params);

/**
 * CTOR consensus constants
 */
namespace consensus {
    /** CTOR version bit for BIP9 signaling */
    static const int CTOR_VERSION_BIT = 28;
    
    /** CTOR activation threshold (95% of blocks in period) */
    static const int CTOR_THRESHOLD = 1916; // 95% of 2016
    
    /** CTOR signaling period (2016 blocks = ~2 weeks) */
    static const int CTOR_PERIOD = 2016;
    
    /** CTOR grace period after activation (1008 blocks = ~1 week) */
    static const int CTOR_GRACE_PERIOD = 1008;
    
    /** CTOR start time (Unix timestamp) */
    static const int64_t CTOR_START_TIME = 1640995200; // 2022-01-01 00:00:00 UTC
    
    /** CTOR timeout (Unix timestamp) */
    static const int64_t CTOR_TIMEOUT = 1672531200; // 2023-01-01 00:00:00 UTC
}

} // namespace ctor

#endif // BITCOINDECENTRAL_SCALING_CTOR_CONSENSUS_H
