# src/core/CMakeLists.txt (temporary safe stub)
if(TARGET corelib)
  message(STATUS "Skipping corelib redefinition (already exists)")
else()
  # When you're ready, define your real library under a unique name, e.g., 'bdc_core'
  add_library(bdc_core INTERFACE)
  target_include_directories(bdc_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
  add_library(corelib ALIAS bdc_core)  # Optional: provide the old name as an ALIAS
endif()
