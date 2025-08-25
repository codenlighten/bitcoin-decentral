#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <iostream>

TEST_CASE("binary boots with --help", "[smoke]") {
    // Test that our node binary can be executed with --help
    int result = std::system("./build/src/node/bitdecentral_node --help > /dev/null 2>&1");
    REQUIRE(result == 0);
}

TEST_CASE("binary boots with --version", "[smoke]") {
    // Test that our node binary can be executed with --version
    int result = std::system("./build/src/node/bitdecentral_node --version > /dev/null 2>&1");
    REQUIRE(result == 0);
}

TEST_CASE("basic arithmetic works", "[smoke]") {
    // Basic sanity check that the test framework works
    REQUIRE(1 + 1 == 2);
    REQUIRE(2 * 3 == 6);
}
