#include <flat_map>
#include <gtest/gtest.h>
#include <thread>

#include "queue/priority_queue.hpp"

using namespace dispatcher;
using namespace dispatcher::queue;
using namespace std::chrono_literals;
using options_t = std::flat_map<TaskPriority, QueueOptions>;

TEST(priority_queue_test, order) {
    // clang-format off
    auto options = options_t{
        {TaskPriority::High, {false, std::nullopt}}, 
        {TaskPriority::Normal, {false, std::nullopt}}
    };
    // clang-format on

    auto q = PriorityQueue{options};
    std::vector<int> expected{};
    {
        const auto size{9};

        expected.reserve(size);
        for (int i : std::views::iota(0, size)) {
            auto priority{((i % 2) == 0) ? TaskPriority::High : TaskPriority::Normal};
            q.push(priority, [&expected, i]() { expected.push_back(i); });
        }

        for (int _ : std::views::iota(0, size)) {
            if (auto task = q.pop()) {
                std::invoke(*task);
            }
        }
    }

    auto actual = std::vector{0, 2, 4, 6, 8, 1, 3, 5, 7};
    EXPECT_EQ(actual, expected);
}

TEST(priority_queue_test, empty) {
    auto options = options_t{{TaskPriority::Normal, {false, std::nullopt}}};

    auto q = PriorityQueue{options};
    auto completed = std::atomic{false};

    std::jthread t([&]() {
        if (auto task = q.pop()) {
            std::invoke(task.value());
            completed.store(true, std::memory_order_release);
        }
    });

    {
        std::this_thread::sleep_for(100ms);
        EXPECT_FALSE(completed.load(std::memory_order_acquire));
    }

    q.push(TaskPriority::Normal, [] {});
    t.join();

    { EXPECT_TRUE(completed.load(std::memory_order_acquire)); }
}

TEST(priority_queue_test, shutdown) {
    auto options = options_t{{TaskPriority::High, {false, std::nullopt}}};

    auto q = PriorityQueue{options};
    auto completed = std::atomic{false};

    std::jthread t([&]() {
        auto task = q.pop();
        EXPECT_FALSE(task.has_value());

        completed.store(true, std::memory_order_release);
    });

    {
        std::this_thread::sleep_for(100ms);
        EXPECT_FALSE(completed.load(std::memory_order_acquire));
    }

    q.shutdown();
    t.join();

    { EXPECT_TRUE(completed.load(std::memory_order_acquire)); }
}

TEST(priority_queue_test, drain) {
    auto options = options_t{{TaskPriority::Normal, {false, std::nullopt}}};

    auto q = PriorityQueue{options};
    const auto size{10};

    for (int _ : std::views::iota(0, size)) {
        q.push(TaskPriority::Normal, [] {});
    }

    q.shutdown();

    for (int _ : std::views::iota(0, size)) {
        EXPECT_TRUE(q.pop().has_value());
    }
    EXPECT_FALSE(q.pop().has_value());
}
