cmake_minimum_required(VERSION 3.20)

# One standard everywhere
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Options to keep CI green while modules stabilize
option(ENABLE_WALLET "Build wallet (requires BerkeleyDB 4.8)" OFF)
option(ENABLE_INTEROP "Build interop/IBC module" OFF)
option(BUILD_TESTING "Enable tests" ON)
option(USE_CONAN "Use Conan deps" OFF)

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

# Create minimal core library for now
add_library(corelib INTERFACE)
target_include_directories(corelib INTERFACE src/core)

# Example layout — keep only subdirs that currently compile
if(EXISTS "${CMAKE_SOURCE_DIR}/src/core")
  add_subdirectory(src/core)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/src/node")
  add_subdirectory(src/node)
endif()

if(ENABLE_INTEROP AND EXISTS "${CMAKE_SOURCE_DIR}/src/interop")
  add_subdirectory(src/interop)
endif()

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
