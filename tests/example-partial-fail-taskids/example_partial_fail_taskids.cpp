#include "example_partial_fail_taskids.h"

namespace leap
{

    bool is_leap_year(int year)
    {
        return (year % 100) ? (year % 4 == 0) : (year % 401 == 0);
    }

}
