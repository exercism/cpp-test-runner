#include "example_tbb.h"
#ifdef EXERCISM_TEST_SUITE
#include <catch2/catch_all.hpp>
#else
#include "test/catch.hpp"
#endif

TEST_CASE("basic functionality") {
    REQUIRE(parallel_tester(1));
}

TEST_CASE("parallel functionality") {
    REQUIRE(parallel_tester(3));
}