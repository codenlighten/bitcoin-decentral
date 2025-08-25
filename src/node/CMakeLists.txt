add_executable(bitdecentral_node
  main.cpp
)

target_include_directories(bitdecentral_node PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/core
)

target_link_libraries(bitdecentral_node PRIVATE
  corelib
)
