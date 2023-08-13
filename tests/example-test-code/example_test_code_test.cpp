#include "example_test_code.h"
#ifdef EXERCISM_TEST_SUITE
#include <catch2/catch_all.hpp>
#else
#include "test/catch.hpp"
#endif

TEST_CASE("not_divisible_by_4")
{
    // some nonsense code to test test cases
    // with several lines

    int a{3};

    if(a > 2)
        REQUIRE(!leap::is_leap_year(2015));
}

#if defined(EXERCISM_RUN_ALL_TESTS)
TEST_CASE("divisible_by_2_not_divisible_by_4")
{
    REQUIRE(!leap::is_leap_year(1970));
}

TEST_CASE("divisible_by_4_not_divisible_by_100")
{
    REQUIRE(leap::is_leap_year(1996));

    // A common problem are double quotes, which might break the json file: "
    // Hopefully two singles quotes are not a problem: ''
}

TEST_CASE("divisible_by_100_not_divisible_by_400")
{
    REQUIRE(!leap::is_leap_year(2100));

    // Test to see if things after the Require macro
    // get erased:
    int b{66};
    if (b < 100) b -= 7;
}

TEST_CASE("divisible_by_400")
{
    REQUIRE(leap::is_leap_year(2000));
}

TEST_CASE("divisible_by_200_not_divisible_by_400")
{
    REQUIRE(!leap::is_leap_year(1800));
}
#endif
