# Minimal node binary so CI has something to build/run.
add_executable(bitdecentral_node
  main.cpp
)

# If a real 'corelib' target exists AND it's not INTERFACE, you can link it later.
# For now, keep the node independent so we don't trip over the existing INTERFACE 'corelib'.
# Example (to enable later):
# if(TARGET corelib)
#   get_target_property(_type corelib TYPE)
#   if(NOT _type STREQUAL "INTERFACE_LIBRARY")
#     target_link_libraries(bitdecentral_node PRIVATE corelib)
#   endif()
# endif()
