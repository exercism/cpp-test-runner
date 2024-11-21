#include "example_tbb.h"

#include <ctime>
#include <vector>
#include <algorithm>
#include <execution>
#include <thread>

void sleep(float seconds)
{
    clock_t startClock = clock();
    float secondsAhead = seconds * CLOCKS_PER_SEC;
    std::this_thread::sleep_for(std::chrono::duration<float>{seconds});
}

bool parallel_tester(int elements) {
    std::atomic<int> ops{};
    std::vector<int> ops_count(elements);
    std::for_each(
        std::execution::par,
        ops_count.begin(),
        ops_count.end(),
        [&](auto&& item)
        {
            item = ops;
            sleep(0.1);
            ++ops;
        });
    for (auto i = 0UL; i < ops_count.size(); ++i) {
        for (auto j = i + 1; j < ops_count.size(); ++j) {
            if (ops_count[i] == ops_count[j]) {
                return true;
            }
        }
    }
    return false;
}