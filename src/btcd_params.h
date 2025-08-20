// Copyright (c) 2025 The Bitcoin Decentral developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_BTCD_PARAMS_H
#define BITCOIN_BTCD_PARAMS_H

#include <consensus/amount.h>
#include <cstdint>


// Bitcoin Decentral Network Parameters
// Generated on 2025-08-19 14:56:35

// Genesis block timestamp and message
extern const char* BTCD_GENESIS_TIMESTAMP;
extern const uint32_t BTCD_GENESIS_TIME;
extern const uint32_t BTCD_GENESIS_NONCE;
extern const uint32_t BTCD_GENESIS_BITS;
extern const CAmount BTCD_GENESIS_REWARD;

// Network magic numbers
extern const uint32_t BTCD_MAINNET_MAGIC;
extern const uint32_t BTCD_TESTNET_MAGIC;
extern const uint32_t BTCD_REGTEST_MAGIC;

// Default ports
extern const uint16_t BTCD_MAINNET_PORT;
extern const uint16_t BTCD_TESTNET_PORT;
extern const uint16_t BTCD_RPC_PORT;
extern const uint16_t BTCD_TESTNET_RPC_PORT;

// Address prefixes
extern const uint8_t BTCD_PUBKEY_ADDRESS;
extern const uint8_t BTCD_SCRIPT_ADDRESS;
extern const uint8_t BTCD_SECRET_KEY;

// Bech32 HRP
extern const char* BTCD_BECH32_HRP;


#endif // BITCOIN_BTCD_PARAMS_H
