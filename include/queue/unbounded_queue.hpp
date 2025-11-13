#pragma once
#include "queue/queue.hpp"
#include <condition_variable>
#include <queue>

namespace dispatcher::queue {

class UnboundedQueue : public IQueue {
    // здесь ваш код
public:
    explicit UnboundedQueue(int capacity);
    ~UnboundedQueue() override;

    void push(Task task) override;
    std::optional<Task> try_pop() override;

private:
    const int capacity_;
    std::queue<Task> queue_;
    std::mutex mutex_;
    std::condition_variable not_empty_cv_;
};

}  // namespace dispatcher::queue