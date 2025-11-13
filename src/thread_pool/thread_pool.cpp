#include "thread_pool/thread_pool.hpp"
#include "logger.hpp"
#include <ranges>
#include <string>

namespace dispatcher::thread_pool {

using namespace std::string_literals;

ThreadPool::ThreadPool(std::shared_ptr<queue::PriorityQueue> queue, size_t thread_count) : queue_(std::move(queue)) {
    threads_.reserve(thread_count);
    std::ranges::for_each(std::views::iota(0u, thread_count),
                          [this](auto) { threads_.emplace_back(&ThreadPool::Worker, this); });
}

ThreadPool::~ThreadPool() { queue_->shutdown(); }

void ThreadPool::Worker() {
    while (auto task = queue_->pop()) {
        try {
            std::invoke(task.value());
        } catch (const std::exception &e) {
            Logger::Get().Log("task threw an exception: "s + std::string(e.what()));
        }
    }
}

}  // namespace dispatcher::thread_pool