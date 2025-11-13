#pragma once
#include "queue/bounded_queue.hpp"
#include "queue/unbounded_queue.hpp"
#include "types.hpp"

#include <atomic>
#include <condition_variable>
#include <flat_map>
#include <mutex>

namespace dispatcher::queue {

class PriorityQueue {
public:
    explicit PriorityQueue(const std::flat_map<TaskPriority, QueueOptions> &options);
    ~PriorityQueue();

    void push(TaskPriority priority, Task task);
    std::optional<Task> pop();
    void shutdown();

private:
    std::flat_map<TaskPriority, std::unique_ptr<IQueue>> queues_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> is_shutdown_{false};
};

}  // namespace dispatcher::queue