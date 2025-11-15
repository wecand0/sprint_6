#include <gtest/gtest.h>
#include <thread>

#include "queue/unbounded_queue.hpp"

using namespace dispatcher::queue;

namespace {
void task() {}
}  // namespace

TEST(unbounded_queue_test, empty) {
    auto q = UnboundedQueue{1'000};

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

TEST(unbounded_queue_test, overflow) {
    const auto capacity{1'000};
    auto q = UnboundedQueue{capacity};

    const auto count{10'000};
    for (int i : std::views::iota(0, count)) {
        if (i < capacity) {
            q.push(task);
        } else {
            // clang-format off
            EXPECT_THROW(
                try {
                    q.push(task);
                } catch (const std::runtime_error &e) {
                    EXPECT_STREQ("queue is full", e.what());
                    EXPECT_GE(i, capacity);
                    throw;
                },
                std::runtime_error
            );
            // clang-format on
        }
    }

    for (int i : std::views::iota(0, count)) {
        auto expected = q.try_pop();
        if (i < capacity) {
            EXPECT_TRUE(expected.has_value());
        } else {
            EXPECT_FALSE(expected.has_value());
        }
    }
}

TEST(unbounded_queue_test, multi_thread) {
    const auto capacity{1'000};
    auto q = UnboundedQueue{capacity};

    std::atomic<int> counter{0};

    auto task_for_push = [&]() {
        for (int _ : std::views::iota(0, 500)) {
            q.push([&counter] { counter.fetch_add(1, std::memory_order_relaxed); });
        }
    };

    auto task_for_pop = [&]() {
        for (int _ : std::views::iota(0, 500)) {
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