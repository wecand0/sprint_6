#include "queue/unbounded_queue.hpp"

namespace dispatcher::queue {

UnboundedQueue::UnboundedQueue(int capacity) : capacity_{capacity} {}

UnboundedQueue::~UnboundedQueue() = default;

void UnboundedQueue::push(Task task) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.size() == capacity_) {
            throw std::runtime_error("queue is full");
        }
        queue_.push(std::move(task));
    }
    not_empty_cv_.notify_one();
}

std::optional<Task> UnboundedQueue::try_pop() {
    std::optional<Task> task{};
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return std::nullopt;
        }
        task = std::move(queue_.front());
        queue_.pop();
    }
    return task;
}

}  // namespace dispatcher::queue