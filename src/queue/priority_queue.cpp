#include "queue/priority_queue.hpp"

#include <algorithm>

namespace dispatcher::queue {

PriorityQueue::PriorityQueue(const std::flat_map<TaskPriority, QueueOptions> &options) {
    std::ranges::for_each(options, [this](const auto &pair) {
        if (const auto &[priority, queue_options] = pair; queue_options.bounded) {
            queues_.emplace(priority, std::make_unique<BoundedQueue>(queue_options.capacity.value_or(1024)));
        } else {
            queues_.emplace(priority, std::make_unique<UnboundedQueue>(queue_options.capacity.value_or(1024)));
        }
    });
}

PriorityQueue::~PriorityQueue() {}

void PriorityQueue::push(TaskPriority priority, Task task) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queues_.contains(priority)) {
            queues_.at(priority)->push(std::move(task));
        }
    }
    cv_.notify_one();
}

void PriorityQueue::shutdown() {
    is_shutdown_.store(true, std::memory_order_release);
    cv_.notify_all();
}

std::optional<Task> PriorityQueue::pop() {
    std::unique_lock<std::mutex> lock(mutex_);

    while (true) {
        for (auto const &[priority, queue] : queues_) {
            if (auto task = queue->try_pop()) {
                return task;
            }
        }

        if (is_shutdown_.load(std::memory_order_acquire)) {
            return std::nullopt;
        }

        cv_.wait(lock);
    }
}

}  // namespace dispatcher::queue