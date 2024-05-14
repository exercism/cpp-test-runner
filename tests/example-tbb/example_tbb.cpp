#include "example_tbb.h"


#include <ctime>
#include <vector>
#include <algorithm>
#include <execution>

using namespace std;


void sleep(float seconds){
    clock_t startClock = clock();
    float secondsAhead = seconds * CLOCKS_PER_SEC;
    // do nothing until the elapsed time has passed.
    while(clock() < startClock+secondsAhead);
    return;
}

bool parallel_tester(int elements) {
    int ops{};
    bool is_par{true};
    std::vector<int> foo(elements);
    std::for_each(
        std::execution::par,
        foo.begin(),
        foo.end(),
        [&](auto&& item)
        {
            item = ops;
            if (ops != 0) {
                is_par = false;
            }
            sleep(0.1);
            ++ops;
        });

    return is_par;
}