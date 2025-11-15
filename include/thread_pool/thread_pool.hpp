#pragma once
#include "queue/priority_queue.hpp"
#include <memory>
#include <thread>
#include <vector>

namespace dispatcher::thread_pool {

class ThreadPool {
public:
    ThreadPool(std::shared_ptr<queue::PriorityQueue> queue, size_t thread_count);
    ~ThreadPool();

private:
    void Worker();

    std::vector<std::jthread> threads_;
    std::shared_ptr<queue::PriorityQueue> queue_;
};

}  // namespace dispatcher::thread_pool
