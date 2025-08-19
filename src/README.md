# BitcoinDecentral Decentral Source Code

This directory will contain the core BitcoinDecentral Decentral (BTCD) source code.

## Current Status

**Phase 0: Project Setup** - The source code will be populated in Phase 1 when we fork from BitcoinDecentral Core.

## Planned Structure (Phase 1)

```
src/
├── consensus/          # Consensus rules and validation
├── crypto/            # Cryptographic primitives
├── net/               # Network protocol implementation
├── rpc/               # RPC server and commands
├── wallet/            # Wallet functionality
├── qt/                # GUI application
├── test/              # Unit tests
├── util/              # Utility functions
├── chainparams.cpp    # Network parameters
├── main.cpp           # Main application entry point
└── ...                # Other BitcoinDecentral Core files (modified)
```

## Key Modifications from BitcoinDecentral Core

The following areas will be modified during development:

### Phase 2: Scaling Features
- **CTOR Implementation** - Transaction ordering in `consensus/`
- **Block Size Removal** - Unbounded blocks in `consensus/validation.cpp`
- **Xthinner Compression** - Block compression in `net/`

### Phase 3: Decentralization Tech
- **ASERT Algorithm** - Difficulty adjustment in `consensus/`
- **Enhanced Opcodes** - Script validation in `script/`
- **UTXO Commitments** - Fast sync in `consensus/`

### Network Parameters
- **Chain Parameters** - Custom network settings in `chainparams.cpp`
- **Address Format** - BTCD address prefix implementation
- **Genesis Block** - Custom genesis block definition
