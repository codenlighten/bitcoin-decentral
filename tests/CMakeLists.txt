include(FetchContent)

# Catch2 v3 header-only
FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(catch2)

add_executable(unit_smoke smoke.cpp)
target_link_libraries(unit_smoke PRIVATE corelib Catch2::Catch2WithMain)

add_test(NAME unit_smoke COMMAND unit_smoke)

# Example of labeling (we run all by default)
set_tests_properties(unit_smoke PROPERTIES LABELS "unit")

# If the node target exists, add a runtime smoke test that calls --help
if(TARGET bitdecentral_node)
  add_test(NAME node_help
           COMMAND $<TARGET_FILE:bitdecentral_node> --help)
  set_tests_properties(node_help PROPERTIES LABELS "smoke")
endif()
