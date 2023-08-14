#include "example_catch2_v2_convert.h"

namespace leap
{

    bool is_leap_year(int year)
    {
        return (year % 100) ? (year % 4 == 0) : (year % 400 == 0);
    }

}
