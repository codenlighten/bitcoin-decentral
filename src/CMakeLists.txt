# Copyright (c) 2023-present The BitcoinDecentral Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or https://opensource.org/license/mit/.

# Windows resources only on Windows and only if helper exists
if(WIN32)
  set(_winres "${CMAKE_SOURCE_DIR}/cmake/AddWindowsResources.cmake")
  if(EXISTS "${_winres}")
    include(AddWindowsResources)
  else()
    message(STATUS "AddWindowsResources.cmake not present; skipping Windows resource step.")
  endif()
endif()

configure_file(${PROJECT_SOURCE_DIR}/cmake/bitcoindecentral-build-config.h.in bitcoindecentral-build-config.h USE_SOURCE_PERMISSIONS @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

#=============================
# Subprojects
#=============================
# Subprojects include subdirectories that do or could have tests
# and/or benchmark binaries, such as all subtrees and univalue.
# These need to be included before CMAKE_*_OUTPUT_DIRECTORY variables
# are set, so output locations of subproject tests and libraries are
# not overridden.

# Helper cmake fragments for third-party libs (crc32c, leveldb, minisketch)
foreach(_frag IN ITEMS "crc32c.cmake" "leveldb.cmake" "minisketch.cmake")
  set(_f "${CMAKE_SOURCE_DIR}/cmake/${_frag}")
  if(EXISTS "${_f}")
    include("${_f}")
  else()
    message(STATUS "Optional helper not found: ${_f} (will rely on system packages or targets already defined).")
  endif()
endforeach()

# If subtrees exist, build them; otherwise rely on system packages.
# crc32c
if(EXISTS "${CMAKE_SOURCE_DIR}/src/crc32c/CMakeLists.txt")
  add_subdirectory("${CMAKE_SOURCE_DIR}/src/crc32c" EXCLUDE_FROM_ALL)
  set(CRC32C_LIB crc32c)
else()
  # Fallback to system library search (most distributions don't package CRC32C)
  find_library(CRC32C_LIB NAMES crc32c HINTS /usr/lib /usr/local/lib)
  if(NOT CRC32C_LIB)
    message(STATUS "CRC32C not found as system library; will build without CRC32C support or rely on subtree.")
    # Allow building without CRC32C if not available
    option(WITH_CRC32C "Enable CRC32C support" OFF)
    if(WITH_CRC32C)
      message(FATAL_ERROR "WITH_CRC32C=ON but no subtree found and no system library available.")
    else()
      add_compile_definitions(BITCOIN_NO_CRC32C=1)
    endif()
  endif()
endif()

# leveldb
if(EXISTS "${CMAKE_SOURCE_DIR}/src/leveldb/CMakeLists.txt")
  add_subdirectory("${CMAKE_SOURCE_DIR}/src/leveldb" EXCLUDE_FROM_ALL)
  set(LEVELDB_LIB leveldb)
else()
  find_library(LEVELDB_LIB NAMES leveldb libleveldb HINTS /usr/lib /usr/local/lib)
endif()

# minisketch (usually subtree; not commonly packaged)
if(EXISTS "${CMAKE_SOURCE_DIR}/src/minisketch/CMakeLists.txt")
  add_subdirectory("${CMAKE_SOURCE_DIR}/src/minisketch" EXCLUDE_FROM_ALL)
  set(MINISKETCH_LIB minisketch)
else()
  # Allow building without minisketch if not available
  option(WITH_MINISKETCH "Enable minisketch-based compact reconciliation" OFF)
  if(WITH_MINISKETCH)
    message(FATAL_ERROR "WITH_MINISKETCH=ON but no subtree found and no system package available.")
  else()
    add_compile_definitions(BITCOIN_NO_MINISKETCH=1)
  endif()
endif()
add_subdirectory(univalue)
#=============================
# secp256k1 subtree
#=============================
message("")

# secp256k1: prefer subtree if present; otherwise system dev package
if(EXISTS "${CMAKE_SOURCE_DIR}/src/secp256k1/CMakeLists.txt")
  message(STATUS "Configuring secp256k1 subtree...")
else()
  find_library(SECP256K1_LIB NAMES secp256k1 HINTS /usr/lib /usr/local/lib)
  if(NOT SECP256K1_LIB)
    message(FATAL_ERROR "secp256k1 not found (no subtree and no system library).")
  endif()
  # Skip subtree configuration if using system library
  return()
endif()
set(SECP256K1_DISABLE_SHARED ON CACHE BOOL "" FORCE)
set(SECP256K1_ENABLE_MODULE_ECDH OFF CACHE BOOL "" FORCE)
set(SECP256K1_ENABLE_MODULE_RECOVERY ON CACHE BOOL "" FORCE)
set(SECP256K1_ENABLE_MODULE_MUSIG OFF CACHE BOOL "" FORCE)
# Do not build secp256k1 tests/bench (they expect wycheproof headers)
set(SECP256K1_BUILD_TESTS OFF CACHE BOOL "Build secp256k1 tests" FORCE)
set(SECP256K1_BUILD_BENCHMARK OFF CACHE BOOL "Build secp256k1 benchmarks" FORCE)
set(SECP256K1_BUILD_EXHAUSTIVE_TESTS OFF CACHE BOOL "Build secp256k1 exhaustive tests" FORCE)
if(NOT BUILD_TESTS)
  # Always skip the ctime tests, if we are building no other tests.
  # Otherwise, they are built if Valgrind is available. See SECP256K1_VALGRIND.
  set(SECP256K1_BUILD_CTIME_TESTS ${BUILD_TESTS} CACHE BOOL "" FORCE)
endif()
set(SECP256K1_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(_gtif "${CMAKE_SOURCE_DIR}/cmake/modules/GetTargetInterface.cmake")
if(EXISTS "${_gtif}")
  include("${_gtif}")
else()
  message(STATUS "GetTargetInterface.cmake missing; defining inline shim.")
  function(get_target_interface tgt OUT_INCLUDES OUT_LIBS)
    if(NOT TARGET ${tgt})
      message(FATAL_ERROR "get_target_interface(): '${tgt}' is not a target")
    endif()
    get_target_property(_incs ${tgt} INTERFACE_INCLUDE_DIRECTORIES)
    if(NOT _incs)
      set(_incs "")
    endif()
    get_target_property(_libs ${tgt} INTERFACE_LINK_LIBRARIES)
    if(NOT _libs)
      set(_libs "")
    endif()
    set(${OUT_INCLUDES} "${_incs}" PARENT_SCOPE)
    set(${OUT_LIBS}     "${_libs}" PARENT_SCOPE)
  endfunction()
endif()
# -fsanitize and related flags apply to both C++ and C,
# so we can pass them down to libsecp256k1 as CFLAGS and LDFLAGS.
# Only extract sanitizer flags if sanitize_interface target exists
set(SECP256K1_APPEND_CFLAGS "")
set(SECP256K1_APPEND_LDFLAGS "")
if(TARGET sanitize_interface)
  get_target_interface(_secp_cflags "" sanitize_interface COMPILE_OPTIONS)
  get_target_interface(_secp_ldflags "" sanitize_interface LINK_OPTIONS)
  set(SECP256K1_APPEND_CFLAGS "${_secp_cflags}")
  set(SECP256K1_APPEND_LDFLAGS "${_secp_ldflags}")
else()
  message(STATUS "sanitize_interface target not found; building secp256k1 without sanitizer flags.")
endif()
string(STRIP "${SECP256K1_APPEND_CFLAGS} ${APPEND_CPPFLAGS}" SECP256K1_APPEND_CFLAGS)
string(STRIP "${SECP256K1_APPEND_CFLAGS} ${APPEND_CFLAGS}" SECP256K1_APPEND_CFLAGS)
set(SECP256K1_APPEND_CFLAGS ${SECP256K1_APPEND_CFLAGS} CACHE STRING "" FORCE)
string(STRIP "${SECP256K1_APPEND_LDFLAGS} ${APPEND_LDFLAGS}" SECP256K1_APPEND_LDFLAGS)
set(SECP256K1_APPEND_LDFLAGS ${SECP256K1_APPEND_LDFLAGS} CACHE STRING "" FORCE)
# We want to build libsecp256k1 with the most tested RelWithDebInfo configuration.
enable_language(C)
foreach(config IN LISTS CMAKE_BUILD_TYPE CMAKE_CONFIGURATION_TYPES)
  if(config STREQUAL "")
    continue()
  endif()
  string(TOUPPER "${config}" config)
  set(CMAKE_C_FLAGS_${config} "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
endforeach()
# If the CFLAGS environment variable is defined during building depends
# and configuring this build system, its content might be duplicated.
if(DEFINED ENV{CFLAGS})
  deduplicate_flags(CMAKE_C_FLAGS)
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)
add_subdirectory(secp256k1)
set_target_properties(secp256k1 PROPERTIES
  EXCLUDE_FROM_ALL TRUE
)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set top-level target output locations.
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()
if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()
if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()

add_custom_target(generate_build_info
  BYPRODUCTS ${PROJECT_BINARY_DIR}/src/bitcoindecentral-build-info.h
  COMMAND ${CMAKE_COMMAND} -DBUILD_INFO_HEADER_PATH=${PROJECT_BINARY_DIR}/src/bitcoindecentral-build-info.h -DSOURCE_DIR=${PROJECT_SOURCE_DIR} -P ${PROJECT_SOURCE_DIR}/cmake/script/GenerateBuildInfo.cmake
  COMMENT "Generating bitcoindecentral-build-info.h"
  VERBATIM
)
add_library(bitcoin_clientversion STATIC EXCLUDE_FROM_ALL
  clientversion.cpp
)
target_link_libraries(bitcoin_clientversion
  PRIVATE
    core_interface
)
add_dependencies(bitcoin_clientversion generate_build_info)

add_subdirectory(crypto)
add_subdirectory(util)
if(WITH_MULTIPROCESS)
  add_subdirectory(ipc)
endif()

add_library(bitcoin_consensus STATIC EXCLUDE_FROM_ALL
  arith_uint256.cpp
  consensus/merkle.cpp
  consensus/tx_check.cpp
  hash.cpp
  primitives/block.cpp
  primitives/transaction.cpp
  pubkey.cpp
  script/interpreter.cpp
  script/script.cpp
  script/script_error.cpp
  uint256.cpp
)
target_link_libraries(bitcoin_consensus
  PRIVATE
    core_interface
    bitcoin_crypto
    secp256k1
)

if(WITH_ZMQ)
  add_subdirectory(zmq)
endif()

# Home for common functionality shared by different executables and libraries.
# Similar to `bitcoin_util` library, but higher-level.
add_library(bitcoin_common STATIC EXCLUDE_FROM_ALL
  addresstype.cpp
  base58.cpp
  bech32.cpp
  btcd_params.cpp
  chain.cpp
  chainparams.cpp
  chainparamsbase.cpp
  coins.cpp
  common/args.cpp
  common/bloom.cpp
  common/config.cpp
  common/init.cpp
  common/interfaces.cpp
  common/messages.cpp
  common/netif.cpp
  common/pcp.cpp
  common/run_command.cpp
  common/settings.cpp
  common/signmessage.cpp
  common/system.cpp
  common/url.cpp
  scaling/ctor/ordering.cpp
  scaling/ctor/validation.cpp
  scaling/ctor/consensus.cpp
  scaling/blocksize/governance.cpp
  scaling/blocksize/validation.cpp
  scaling/xthinner/compression_simple.cpp
  scaling/xthinner/network_simple.cpp
  scaling/mempool/advanced.cpp
  smartcontracts/vm.cpp
    # Hybrid Consensus System (Phase 3.2)
    consensus/hybrid.cpp
    consensus/governance.cpp
    # Cross-Chain Interoperability (Phase 3.3)
    interop/bridges.cpp
    interop/ibc.cpp
    crypto/quantum.cpp
  compressor.cpp
  core_read.cpp
  core_write.cpp
  deploymentinfo.cpp
  external_signer.cpp
  init/common.cpp
  kernel/chainparams.cpp
  key.cpp
  key_io.cpp
  merkleblock.cpp
  net_permissions.cpp
  net_types.cpp
  netaddress.cpp
  netbase.cpp
  outputtype.cpp
  policy/feerate.cpp
  policy/policy.cpp
  pow.cpp
  protocol.cpp
  psbt.cpp
  rpc/rawtransaction_util.cpp
  rpc/request.cpp
  rpc/util.cpp
  scheduler.cpp
  script/descriptor.cpp
  script/miniscript.cpp
  script/parsing.cpp
  script/sign.cpp
  script/signingprovider.cpp
  script/solver.cpp
)
target_link_libraries(bitcoin_common
  PRIVATE
    core_interface
    bitcoin_consensus
    bitcoin_util
    univalue
    secp256k1
    Boost::headers
    $<TARGET_NAME_IF_EXISTS:USDT::headers>
    $<$<PLATFORM_ID:Windows>:ws2_32>
)

include(InstallBinaryComponent)

if(ENABLE_WALLET)
  add_subdirectory(wallet)

  if(BUILD_WALLET_TOOL)
    add_executable(bitcoindecentral-wallet
      bitcoindecentral-wallet.cpp
      init/bitcoindecentral-wallet.cpp
      wallet/wallettool.cpp
    )
    add_windows_resources(bitcoindecentral-wallet bitcoindecentral-wallet-res.rc)
    target_link_libraries(bitcoindecentral-wallet
      core_interface
      bitcoin_wallet
      bitcoin_common
      bitcoin_util
      Boost::headers
    )
    install_binary_component(bitcoindecentral-wallet HAS_MANPAGE)
  endif()
endif()


# P2P and RPC server functionality used by `bitcoindecentrald` and `bitcoindecentral-qt` executables.
add_library(bitcoin_node STATIC EXCLUDE_FROM_ALL
  addrdb.cpp
  addrman.cpp
  banman.cpp
  bip324.cpp
  blockencodings.cpp
  blockfilter.cpp
  consensus/tx_verify.cpp
  dbwrapper.cpp
  deploymentstatus.cpp
  flatfile.cpp
  headerssync.cpp
  httprpc.cpp
  httpserver.cpp
  i2p.cpp
  index/base.cpp
  index/blockfilterindex.cpp
  index/coinstatsindex.cpp
  index/txindex.cpp
  init.cpp
  kernel/chain.cpp
  kernel/checks.cpp
  kernel/coinstats.cpp
  kernel/context.cpp
  kernel/cs_main.cpp
  kernel/disconnected_transactions.cpp
  kernel/mempool_removal_reason.cpp
  mapport.cpp
  net.cpp
  net_processing.cpp
  netgroup.cpp
  node/abort.cpp
  node/blockmanager_args.cpp
  node/blockstorage.cpp
  node/caches.cpp
  node/chainstate.cpp
  node/chainstatemanager_args.cpp
  node/coin.cpp
  node/coins_view_args.cpp
  node/connection_types.cpp
  node/context.cpp
  node/database_args.cpp
  node/eviction.cpp
  node/interface_ui.cpp
  node/interfaces.cpp
  node/kernel_notifications.cpp
  node/mempool_args.cpp
  node/mempool_persist.cpp
  node/mempool_persist_args.cpp
  node/miner.cpp
  node/mini_miner.cpp
  node/minisketchwrapper.cpp
  node/peerman_args.cpp
  node/psbt.cpp
  node/timeoffsets.cpp
  node/transaction.cpp
  node/txdownloadman_impl.cpp
  node/txreconciliation.cpp
  node/utxo_snapshot.cpp
  node/warnings.cpp
  noui.cpp
  policy/ephemeral_policy.cpp
  policy/fees.cpp
  policy/fees_args.cpp
  policy/packages.cpp
  policy/rbf.cpp
  policy/settings.cpp
  policy/truc_policy.cpp
  rest.cpp
  rpc/blockchain.cpp
  rpc/external_signer.cpp
  rpc/fees.cpp
  rpc/mempool.cpp
  rpc/mining.cpp
  rpc/net.cpp
  rpc/node.cpp
  rpc/output_script.cpp
  rpc/rawtransaction.cpp
  rpc/server.cpp
  rpc/server_util.cpp
  rpc/signmessage.cpp
  rpc/txoutproof.cpp
  script/sigcache.cpp
  signet.cpp
  torcontrol.cpp
  txdb.cpp
  txmempool.cpp
  txorphanage.cpp
  txrequest.cpp
  validation.cpp
  validationinterface.cpp
  versionbits.cpp
  $<$<TARGET_EXISTS:bitcoin_wallet>:wallet/init.cpp>
  $<$<NOT:$<TARGET_EXISTS:bitcoin_wallet>>:dummywallet.cpp>
)
target_link_libraries(bitcoin_node
  PRIVATE
    core_interface
    bitcoin_common
    bitcoin_util
    $<TARGET_NAME_IF_EXISTS:bitcoin_zmq>
    leveldb
    minisketch
    univalue
    Boost::headers
    $<TARGET_NAME_IF_EXISTS:libevent::core>
    $<TARGET_NAME_IF_EXISTS:libevent::extra>
    $<TARGET_NAME_IF_EXISTS:libevent::pthreads>
    $<TARGET_NAME_IF_EXISTS:USDT::headers>
)


# BitcoinDecentral Core bitcoindecentrald.
if(BUILD_DAEMON)
  add_executable(bitcoindecentrald
    bitcoindecentrald.cpp
    init/bitcoindecentrald.cpp
  )
  add_windows_resources(bitcoindecentrald bitcoindecentrald-res.rc)
  target_link_libraries(bitcoindecentrald
    core_interface
    bitcoin_node
    $<TARGET_NAME_IF_EXISTS:bitcoin_wallet>
  )
  install_binary_component(bitcoindecentrald HAS_MANPAGE)
endif()
if(WITH_MULTIPROCESS AND BUILD_DAEMON)
  add_executable(bitcoindecentral-node
    bitcoindecentrald.cpp
    init/bitcoindecentral-node.cpp
  )
  target_link_libraries(bitcoindecentral-node
    core_interface
    bitcoin_node
    bitcoin_ipc
    $<TARGET_NAME_IF_EXISTS:bitcoin_wallet>
  )
  install_binary_component(bitcoindecentral-node)
endif()

if(WITH_MULTIPROCESS AND BUILD_TESTS)
    # bitcoin_ipc_test library target is defined here in src/CMakeLists.txt
    # instead of src/test/CMakeLists.txt so capnp files in src/test/ are able to
    # reference capnp files in src/ipc/capnp/ by relative path. The Cap'n Proto
    # compiler only allows importing by relative path when the importing and
    # imported files are underneath the same compilation source prefix, so the
    # source prefix must be src/, not src/test/
    add_library(bitcoin_ipc_test STATIC EXCLUDE_FROM_ALL
      test/ipc_test.cpp
    )
    target_capnp_sources(bitcoin_ipc_test ${PROJECT_SOURCE_DIR}
      test/ipc_test.capnp
    )
    add_dependencies(bitcoin_ipc_test bitcoin_ipc_headers)
endif()


add_library(bitcoin_cli STATIC EXCLUDE_FROM_ALL
  compat/stdin.cpp
  rpc/client.cpp
)
target_link_libraries(bitcoin_cli
  PUBLIC
    core_interface
    univalue
)

# Large Block Stress Test Tool
add_executable(large_block_stress_test
  test/large_block_stress_test.cpp
)
target_link_libraries(large_block_stress_test
  PRIVATE
    core_interface
    bitcoin_common
    bitcoin_util
    bitcoin_consensus
    univalue
    secp256k1
    Boost::headers
)


# BitcoinDecentral Core RPC client
if(BUILD_CLI)
  add_executable(bitcoindecentral-cli bitcoindecentral-cli.cpp)
  add_windows_resources(bitcoindecentral-cli bitcoindecentral-cli-res.rc)
  target_link_libraries(bitcoindecentral-cli
    core_interface
    bitcoin_cli
    bitcoin_common
    bitcoin_util
    libevent::core
    libevent::extra
  )
  install_binary_component(bitcoindecentral-cli HAS_MANPAGE)
endif()


if(BUILD_TX)
  add_executable(bitcoindecentral-tx bitcoindecentral-tx.cpp)
  add_windows_resources(bitcoindecentral-tx bitcoindecentral-tx-res.rc)
  target_link_libraries(bitcoindecentral-tx
    core_interface
    bitcoin_common
    bitcoin_util
    univalue
  )
  install_binary_component(bitcoindecentral-tx HAS_MANPAGE)
endif()


if(BUILD_UTIL)
  add_executable(bitcoindecentral-util bitcoindecentral-util.cpp)
  add_windows_resources(bitcoindecentral-util bitcoindecentral-util-res.rc)
  target_link_libraries(bitcoindecentral-util
    core_interface
    bitcoin_common
    bitcoin_util
  )
  install_binary_component(bitcoindecentral-util HAS_MANPAGE)
endif()


if(BUILD_GUI)
  add_subdirectory(qt)
endif()


if(BUILD_KERNEL_LIB)
  add_subdirectory(kernel)
endif()

if(BUILD_UTIL_CHAINSTATE)
  add_executable(bitcoindecentral-chainstate
    bitcoindecentral-chainstate.cpp
  )
  # TODO: The `SKIP_BUILD_RPATH` property setting can be deleted
  #       in the future after reordering Guix script commands to
  #       perform binary checks after the installation step.
  # Relevant discussions:
  # - https://github.com/hebasto/bitcoindecentral/pull/236#issuecomment-2183120953
  # - https://github.com/bitcoindecentral/bitcoindecentral/pull/30312#issuecomment-2191235833
  set_target_properties(bitcoindecentral-chainstate PROPERTIES
    SKIP_BUILD_RPATH OFF
  )
  target_link_libraries(bitcoindecentral-chainstate
    PRIVATE
      core_interface
      bitcoinkernel
  )
endif()


add_subdirectory(test/util)
if(BUILD_BENCH)
  add_subdirectory(bench)
endif()

if(BUILD_TESTS)
  add_subdirectory(test)
endif()

if(BUILD_FUZZ_BINARY)
  add_subdirectory(test/fuzz)
endif()
