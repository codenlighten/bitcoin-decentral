cmake_minimum_required(VERSION 3.16)

project(BitcoinDecentral
    VERSION 0.1.0
    DESCRIPTION "Bitcoin Decentral (BTCD) - A Bitcoin fork focused on scaling and decentralization"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_TESTS "Build test suite" ON)
option(WITH_GUI "Build GUI application" OFF)
option(ENABLE_WALLET "Enable wallet functionality" ON)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    else()
        add_compile_options(-O2)
    endif()
endif()

if(MSVC)
    add_compile_options(/W4)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(/Od)
    else()
        add_compile_options(/O2)
    endif()
endif()

# Find required packages
find_package(Boost REQUIRED COMPONENTS system filesystem thread chrono)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)

# Find optional packages
pkg_check_modules(LIBEVENT libevent)
pkg_check_modules(ZMQ libzmq)

if(WITH_GUI)
    find_package(Qt5 COMPONENTS Core Widgets Network REQUIRED)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files (placeholder - will be populated in Phase 1)
set(BTCD_SOURCES
    # Core sources will be added when we fork Bitcoin Core
    # src/main.cpp
    # src/init.cpp
    # ... other sources
)

# For now, create a placeholder executable to test the build system
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    add_executable(bitcoindecentrald ${BTCD_SOURCES})
    
    target_link_libraries(bitcoindecentrald
        ${Boost_LIBRARIES}
        OpenSSL::SSL
        OpenSSL::Crypto
    )
    
    if(LIBEVENT_FOUND)
        target_link_libraries(bitcoindecentrald ${LIBEVENT_LIBRARIES})
    endif()
    
    if(ZMQ_FOUND)
        target_link_libraries(bitcoindecentrald ${ZMQ_LIBRARIES})
    endif()
else()
    # Create a minimal test target for CI validation
    add_custom_target(validate_build
        COMMAND ${CMAKE_COMMAND} -E echo "Build system validation successful"
        COMMENT "Validating build configuration"
    )
endif()

# Testing
if(BUILD_TESTS)
    enable_testing()
    
    # Add test directories when available
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/test")
        add_subdirectory(src/test)
    endif()
    
    # For now, add a basic validation test
    add_test(
        NAME build_system_test
        COMMAND ${CMAKE_COMMAND} -E echo "Build system test passed"
    )
endif()

# Installation
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    install(TARGETS bitcoindecentrald
        RUNTIME DESTINATION bin
    )
endif()

# Documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs)
    set(DOXYGEN_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/src)
    
    doxygen_add_docs(docs
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        COMMENT "Generate API documentation"
    )
endif()

# Print configuration summary
message(STATUS "Bitcoin Decentral Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
message(STATUS "  Build GUI: ${WITH_GUI}")
message(STATUS "  Enable wallet: ${ENABLE_WALLET}")
message(STATUS "  Boost version: ${Boost_VERSION}")
message(STATUS "  OpenSSL version: ${OPENSSL_VERSION}")

if(LIBEVENT_FOUND)
    message(STATUS "  LibEvent: Found")
else()
    message(STATUS "  LibEvent: Not found")
endif()

if(ZMQ_FOUND)
    message(STATUS "  ZeroMQ: Found")
else()
    message(STATUS "  ZeroMQ: Not found")
endif()
