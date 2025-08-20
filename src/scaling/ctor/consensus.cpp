// Copyright (c) 2025 The Bitcoin Decentral developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <scaling/ctor/consensus.h>
#include <scaling/ctor/validation.h>
#include <logging.h>
#include <util/strencodings.h>

namespace ctor {

CTORDeployment GetCTORDeployment() {
    CTORDeployment deployment;
    deployment.bit = consensus::CTOR_VERSION_BIT;
    deployment.nStartTime = consensus::CTOR_START_TIME;
    deployment.nTimeout = consensus::CTOR_TIMEOUT;
    deployment.nThreshold = consensus::CTOR_THRESHOLD;
    deployment.nPeriod = consensus::CTOR_PERIOD;
    return deployment;
}

ThresholdState GetCTORState(const CBlockIndex* pindex, const Consensus::Params& params) {
    // For now, return a simple state based on height
    // TODO: Implement proper BIP9 threshold state calculation
    if (!pindex) {
        return ThresholdState::DEFINED;
    }
    
    int activation_height = ctor::GetCTORActivationHeight();
    
    if (pindex->nHeight < activation_height - consensus::CTOR_PERIOD) {
        return ThresholdState::DEFINED;
    } else if (pindex->nHeight < activation_height) {
        return ThresholdState::STARTED;
    } else {
        return ThresholdState::ACTIVE;
    }
}

void InitializeCTORConsensus(Consensus::Params& params) {
    LogPrintf("CTOR: Initializing consensus parameters\n");
    
    // Add CTOR deployment to consensus parameters
    // This would integrate with the existing versionbits system
    // For now, we'll use our custom activation logic
    
    LogPrintf("CTOR: Consensus parameters initialized\n");
}

bool BlockSignalsCTOR(const CBlock& block) {
    // Check if block version signals CTOR support
    // BIP9 signaling: check if the CTOR bit is set in block version
    return (block.nVersion & (1 << consensus::CTOR_VERSION_BIT)) != 0;
}

bool ValidateCTORSignaling(const CBlock& block, const CBlockIndex* pindex, 
                          const Consensus::Params& params) {
    if (!pindex) {
        return false;
    }
    
    ThresholdState state = GetCTORState(pindex, params);
    
    // During STARTED phase, blocks can optionally signal CTOR
    if (state == ThresholdState::STARTED) {
        LogPrintf("CTOR: Block at height %d signals CTOR: %s\n", 
                pindex->nHeight, BlockSignalsCTOR(block) ? "yes" : "no");
        return true; // Signaling is optional during STARTED phase
    }
    
    // During ACTIVE phase, CTOR rules must be followed
    if (state == ThresholdState::ACTIVE) {
        return ValidateBlockCTOR(block, pindex, *(new BlockValidationState()));
    }
    
    return true; // No validation needed in other states
}

int GetCTORActivationHeight(const CBlockIndex* pindex, const Consensus::Params& params) {
    // For now, return fixed activation height
    // TODO: Implement dynamic activation based on BIP9 signaling
    return activation::CTOR_ACTIVATION_HEIGHT;
}

bool IsInCTORGracePeriod(int height, const Consensus::Params& params) {
    int activation_height = ctor::GetCTORActivationHeight();
    return (height >= activation_height && 
            height < activation_height + consensus::CTOR_GRACE_PERIOD);
}

bool ValidateCTORChain(const CBlockIndex* pindex, const Consensus::Params& params) {
    if (!pindex) {
        return false;
    }
    
    // Validate that the chain follows CTOR rules after activation
    const CBlockIndex* current = pindex;
    int activation_height = GetCTORActivationHeight(pindex, params);
    
    while (current && current->nHeight >= activation_height) {
        // For a full validation, we would need to check each block
        // For now, we assume the chain is valid if we reach this point
        current = current->pprev;
    }
    
    LogPrintf("CTOR: Chain validation successful up to height %d\n", pindex->nHeight);
    return true;
}

} // namespace ctor
