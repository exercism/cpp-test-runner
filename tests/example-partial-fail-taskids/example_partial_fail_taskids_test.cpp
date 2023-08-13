#include "example_partial_fail_taskids.h"
#ifdef EXERCISM_TEST_SUITE
#include <catch2/catch_all.hpp>
#else
#include "test/catch.hpp"
#endif

TEST_CASE("not_divisible_by_4", "[task_1]")
{
    REQUIRE(!leap::is_leap_year(2015));
}

#if defined(EXERCISM_RUN_ALL_TESTS)
TEST_CASE("divisible_by_2_not_divisible_by_4", "[task_1]")
{
    REQUIRE(!leap::is_leap_year(1970));
}

TEST_CASE("divisible_by_4_not_divisible_by_100", "[task_2]")
{
    REQUIRE(leap::is_leap_year(1996));
}

TEST_CASE("divisible_by_100_not_divisible_by_400", "[task_3]")
{
    REQUIRE(!leap::is_leap_year(2100));
}

TEST_CASE("divisible_by_400", "[task_3]")
{
    REQUIRE(leap::is_leap_year(2000));
}

TEST_CASE("divisible_by_200_not_divisible_by_400", "[task_3]")
{
    REQUIRE(!leap::is_leap_year(1800));
}
#endif