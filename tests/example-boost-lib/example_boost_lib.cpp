#include "example_boost_lib.h"

namespace gigasecond
{
  ptime advance(ptime from)
  {
    seconds GIGASECOND = seconds(1000000000);

    ptime result = ptime(from) + seconds(GIGASECOND);
    return result;
  }

}
