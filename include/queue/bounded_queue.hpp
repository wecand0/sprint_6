#pragma once
#include "queue/queue.hpp"
#include <condition_variable>
#include <queue>

namespace dispatcher::queue {

class BoundedQueue : public IQueue {
public:
    explicit BoundedQueue(int capacity);
    ~BoundedQueue() override;

    void push(std::function<void()> task) override;
    std::optional<std::function<void()>> try_pop() override;

private:
    const int capacity_;
    std::queue<Task> queue_;
    std::mutex mutex_;
    std::condition_variable not_full_cv_;
    std::condition_variable not_empty_cv_;
};

}  // namespace dispatcher::queue