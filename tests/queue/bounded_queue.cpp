#include <algorithm>
#include <gtest/gtest.h>
#include <ranges>
#include <thread>

#include "queue/bounded_queue.hpp"

using namespace dispatcher::queue;

namespace {
void task() {}
}  // namespace

TEST(bounded_queue_test, empty) {
    auto q = BoundedQueue{3};

    {
        q.push(task);
        auto expected = q.try_pop();
        EXPECT_TRUE(expected.has_value());
    }

    {
        auto expected = q.try_pop();
        EXPECT_FALSE(expected.has_value());
    }
}

TEST(bounded_queue_test, overflow) {
    const auto capacity{3};
    auto q = BoundedQueue{capacity};

    std::ranges::for_each(std::views::iota(0, capacity), [&q](int) { q.push(task); });
    {
        std::jthread pusher([&q]() { q.push(task); });

        auto expected = q.try_pop();
        EXPECT_TRUE(expected.has_value());
    }

    std::ranges::for_each(std::views::iota(0, capacity), [&q](int) {
        auto expected = q.try_pop();
        EXPECT_TRUE(expected.has_value());
    });

    {
        auto expected = q.try_pop();
        EXPECT_FALSE(expected.has_value());
    }
}

TEST(bounded_queue_test, multi_thread) {
    const auto capacity{100};
    auto q = BoundedQueue{capacity};

    std::atomic<int> counter{0};

    auto task_for_push = [&]() {
        for (int _ : std::views::iota(0, 50)) {
            q.push([&counter] { counter.fetch_add(1, std::memory_order_relaxed); });
        }
    };

    auto task_for_pop = [&]() {
        for (int _ : std::views::iota(0, 50)) {
            while (true) {
                if (auto task = q.try_pop()) {
                    std::invoke(*task);
                    counter.fetch_sub(1, std::memory_order_relaxed);
                    break;
                }
                std::this_thread::yield();
            }
        }
    };

    {
        std::vector<std::jthread> workers;
        workers.reserve(4);
        workers.emplace_back(task_for_pop);
        workers.emplace_back(task_for_pop);
        workers.emplace_back(task_for_push);
        workers.emplace_back(task_for_push);
    }

    EXPECT_EQ(counter, 0);

    {
        auto expected = q.try_pop();
        EXPECT_FALSE(expected.has_value());
    }
}