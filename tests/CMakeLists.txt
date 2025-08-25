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
