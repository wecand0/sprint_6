#include <gtest/gtest.h>

#include "task_dispatcher.hpp"

using namespace dispatcher;
using namespace std::chrono_literals;

TEST(task_dispatcher_test, execute) {
    auto d = TaskDispatcher{4};

    const auto size{100u};
    auto counter = std::atomic{0u};

    {
        std::mutex m;
        std::condition_variable cv;

        for (int i : std::views::iota(0u, size)) {
            auto priority{((i % 2) == 0) ? TaskPriority::High : TaskPriority::Normal};
            d.schedule(priority, [&]() {
                counter.fetch_add(1, std::memory_order_relaxed);
                cv.notify_one();
            });
        }

        std::unique_lock<std::mutex> lock(m);
        cv.wait_for(lock, 1s, [&]() { return counter == size; });
    }

    EXPECT_EQ(counter, size);
}

TEST(task_dispatcher_test, order) {
    auto d = TaskDispatcher{1};

    const auto size{9u};
    std::vector<int> expected{};

    {
        std::mutex m;
        std::condition_variable cv;

        expected.reserve(size);
        for (int i : std::views::iota(0u, size)) {
            auto priority{((i % 2) == 0) ? TaskPriority::High : TaskPriority::Normal};
            d.schedule(priority, [&, priority, value = i]() {
                {
                    std::lock_guard<std::mutex> lock(m);
                    expected.push_back(value);
                }
                cv.notify_one();
            });
        }

        std::this_thread::sleep_for(100ms);

        std::unique_lock<std::mutex> lock(m);
        cv.wait_for(lock, 1s, [&]() { return expected.size() == size; });
    }

    auto actual = std::vector{0, 2, 4, 6, 8, 1, 3, 5, 7};
    EXPECT_EQ(actual, expected);
}
