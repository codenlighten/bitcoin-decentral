# Minimal core library
add_library(corelib INTERFACE)
target_include_directories(corelib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Add a simple header for now
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/version.h")
  file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/version.h" 
    "#pragma once\n#define BITCOIN_DECENTRAL_VERSION \"0.1.0\"\n")
endif()
