#include "compile_error.h"
#include "test/catch.hpp"

using namespace std;

TEST_CASE("Test add two numbers")
{
    REQUIRE(2 == add(1, 1));
}
