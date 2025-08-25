cmake_minimum_required(VERSION 3.20)

# One standard everywhere
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# CI-stability toggles (safe defaults)
option(ENABLE_WALLET "Build wallet (requires BerkeleyDB 4.8)" OFF)
option(ENABLE_INTEROP "Build interop/IBC module" OFF)
option(BUILD_TESTING "Enable tests" ON)
option(USE_CONAN "Use Conan deps" OFF)
option(ENABLE_CORE "Build core library targets" OFF)  # NEW: avoid colliding with existing interface 'corelib'

project(BitcoinDecentral VERSION 0.1.0 LANGUAGES CXX C ASM)

include(CTest)

# Conan integration (optional)
if(USE_CONAN)
  include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL)
  find_package(OpenSSL REQUIRED)
  find_package(Boost REQUIRED COMPONENTS filesystem program_options)
  find_package(ZeroMQ REQUIRED)
  find_package(leveldb REQUIRED)
  find_package(SQLite3 REQUIRED)
  find_package(Libevent REQUIRED)
  find_package(secp256k1 REQUIRED)
  find_package(crc32c REQUIRED)
endif()

# Only add 'src/core' when we're ready (and when it doesn't collide)
if(ENABLE_CORE)
  add_subdirectory(src/core)
endif()

add_subdirectory(src/node)

if(ENABLE_INTEROP AND EXISTS "${CMAKE_SOURCE_DIR}/src/interop")
  add_subdirectory(src/interop)
endif()

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
