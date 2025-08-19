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
const char* BTCD_GENESIS_TIMESTAMP = "Bitcoin Decentral 19/Aug/2025 Scaling Bitcoin with Decentralization";
const uint32_t BTCD_GENESIS_TIME = 1755629795;
const uint32_t BTCD_GENESIS_NONCE = 0; // Will be mined
const uint32_t BTCD_GENESIS_BITS = 0x1d00ffff;
const CAmount BTCD_GENESIS_REWARD = 5000000000;

// Network magic numbers
const uint32_t BTCD_MAINNET_MAGIC = 0xd9b4bef9;
const uint32_t BTCD_TESTNET_MAGIC = 0xdab5bffa;
const uint32_t BTCD_REGTEST_MAGIC = 0xdcb7c1fc;

// Default ports
const uint16_t BTCD_MAINNET_PORT = 8433;
const uint16_t BTCD_TESTNET_PORT = 18433;
const uint16_t BTCD_RPC_PORT = 8432;
const uint16_t BTCD_TESTNET_RPC_PORT = 18432;

// Address prefixes
const uint8_t BTCD_PUBKEY_ADDRESS = 25;
const uint8_t BTCD_SCRIPT_ADDRESS = 85;
const uint8_t BTCD_SECRET_KEY = 153;

// Bech32 HRP
const char* BTCD_BECH32_HRP = "btcd";


#endif // BITCOIN_BTCD_PARAMS_H
