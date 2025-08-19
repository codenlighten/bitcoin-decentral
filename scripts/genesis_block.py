#!/usr/bin/env python3
"""
Bitcoin Decentral Genesis Block Generator
Creates a unique genesis block for the BTCD network
"""

import hashlib
import struct
import time
from datetime import datetime

def sha256d(data):
    """Double SHA256 hash"""
    return hashlib.sha256(hashlib.sha256(data).digest()).digest()

def create_genesis_block():
    """Create Bitcoin Decentral genesis block parameters"""
    
    # Bitcoin Decentral genesis block timestamp and message
    timestamp = "Bitcoin Decentral 19/Aug/2025 Scaling Bitcoin with Decentralization"
    genesis_time = int(time.time())  # Current timestamp
    
    print("Bitcoin Decentral Genesis Block Generator")
    print("=" * 50)
    print(f"Timestamp message: {timestamp}")
    print(f"Genesis time: {genesis_time} ({datetime.fromtimestamp(genesis_time)})")
    
    # Network magic numbers for Bitcoin Decentral
    # These must be unique to prevent cross-network connections
    mainnet_magic = 0xd9b4bef9  # BTCD mainnet magic
    testnet_magic = 0xdab5bffa  # BTCD testnet magic
    regtest_magic = 0xdcb7c1fc  # BTCD regtest magic
    
    print(f"\nNetwork Magic Numbers:")
    print(f"Mainnet: 0x{mainnet_magic:08x}")
    print(f"Testnet: 0x{testnet_magic:08x}")
    print(f"Regtest: 0x{regtest_magic:08x}")
    
    # Default ports for Bitcoin Decentral
    mainnet_port = 8433  # Different from Bitcoin's 8333
    testnet_port = 18433  # Different from Bitcoin's 18333
    rpc_port = 8432      # Different from Bitcoin's 8332
    testnet_rpc_port = 18432  # Different from Bitcoin's 18332
    
    print(f"\nDefault Ports:")
    print(f"Mainnet P2P: {mainnet_port}")
    print(f"Testnet P2P: {testnet_port}")
    print(f"Mainnet RPC: {rpc_port}")
    print(f"Testnet RPC: {testnet_rpc_port}")
    
    # Address prefixes for Bitcoin Decentral
    # Using different prefixes to distinguish BTCD addresses
    pubkey_address_prefix = 25    # BTCD addresses start with 'B'
    script_address_prefix = 85    # BTCD script addresses start with 'b'
    secret_key_prefix = 153       # BTCD private keys
    
    print(f"\nAddress Prefixes:")
    print(f"Pubkey Address: {pubkey_address_prefix} (addresses start with 'B')")
    print(f"Script Address: {script_address_prefix} (script addresses start with 'b')")
    print(f"Secret Key: {secret_key_prefix}")
    
    # Bech32 HRP (Human Readable Part) for Bitcoin Decentral
    bech32_hrp = "btcd"  # Bitcoin Decentral addresses will use 'btcd' prefix
    
    print(f"Bech32 HRP: {bech32_hrp}")
    
    # Genesis block parameters
    genesis_version = 1
    genesis_bits = 0x1d00ffff  # Same initial difficulty as Bitcoin
    genesis_reward = 50 * 100000000  # 50 BTCD (in satoshis)
    
    print(f"\nGenesis Block Parameters:")
    print(f"Version: {genesis_version}")
    print(f"Bits: 0x{genesis_bits:08x}")
    print(f"Reward: {genesis_reward / 100000000} BTCD")
    
    # Generate C++ code for chainparams
    cpp_code = f'''
// Bitcoin Decentral Network Parameters
// Generated on {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}

// Genesis block timestamp and message
const char* BTCD_GENESIS_TIMESTAMP = "{timestamp}";
const uint32_t BTCD_GENESIS_TIME = {genesis_time};
const uint32_t BTCD_GENESIS_NONCE = 0; // Will be mined
const uint32_t BTCD_GENESIS_BITS = 0x{genesis_bits:08x};
const CAmount BTCD_GENESIS_REWARD = {genesis_reward};

// Network magic numbers
const uint32_t BTCD_MAINNET_MAGIC = 0x{mainnet_magic:08x};
const uint32_t BTCD_TESTNET_MAGIC = 0x{testnet_magic:08x};
const uint32_t BTCD_REGTEST_MAGIC = 0x{regtest_magic:08x};

// Default ports
const uint16_t BTCD_MAINNET_PORT = {mainnet_port};
const uint16_t BTCD_TESTNET_PORT = {testnet_port};
const uint16_t BTCD_RPC_PORT = {rpc_port};
const uint16_t BTCD_TESTNET_RPC_PORT = {testnet_rpc_port};

// Address prefixes
const uint8_t BTCD_PUBKEY_ADDRESS = {pubkey_address_prefix};
const uint8_t BTCD_SCRIPT_ADDRESS = {script_address_prefix};
const uint8_t BTCD_SECRET_KEY = {secret_key_prefix};

// Bech32 HRP
const char* BTCD_BECH32_HRP = "{bech32_hrp}";
'''
    
    # Save parameters to file
    with open('/home/greg/Documents/dev/bitcoin-dct2/src/btcd_params.h', 'w') as f:
        f.write(f'''// Copyright (c) 2025 The Bitcoin Decentral developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_BTCD_PARAMS_H
#define BITCOIN_BTCD_PARAMS_H

#include <consensus/amount.h>
#include <cstdint>

{cpp_code}

#endif // BITCOIN_BTCD_PARAMS_H
''')
    
    print(f"\n✅ Parameters saved to src/btcd_params.h")
    
    return {
        'timestamp': timestamp,
        'genesis_time': genesis_time,
        'mainnet_magic': mainnet_magic,
        'testnet_magic': testnet_magic,
        'regtest_magic': regtest_magic,
        'mainnet_port': mainnet_port,
        'testnet_port': testnet_port,
        'rpc_port': rpc_port,
        'testnet_rpc_port': testnet_rpc_port,
        'pubkey_address_prefix': pubkey_address_prefix,
        'script_address_prefix': script_address_prefix,
        'secret_key_prefix': secret_key_prefix,
        'bech32_hrp': bech32_hrp,
        'genesis_bits': genesis_bits,
        'genesis_reward': genesis_reward
    }

if __name__ == "__main__":
    params = create_genesis_block()
    print("\n🚀 Bitcoin Decentral network parameters generated successfully!")
    print("Next steps:")
    print("1. Update kernel/chainparams.cpp with new parameters")
    print("2. Mine genesis block with correct nonce")
    print("3. Update chainparamsbase.cpp with new ports")
    print("4. Test compilation and network isolation")
