#pragma once

#include <flat_map>
#include <memory>

#include "queue/priority_queue.hpp"
#include "thread_pool/thread_pool.hpp"
#include "types.hpp"

namespace dispatcher {

class TaskDispatcher {
    static constexpr std::flat_map<TaskPriority, queue::QueueOptions> get_default_tasks() {
        return {{TaskPriority::High, {true, 1000}}, {TaskPriority::Normal, {false, std::nullopt}}};
    };

public:
    explicit TaskDispatcher(size_t thread_count, const std::flat_map<TaskPriority, queue::QueueOptions> &tasks_options =
                                                     get_default_tasks());
    ~TaskDispatcher();

    void schedule(TaskPriority priority, std::function<void()> task) const;

private:
    std::shared_ptr<queue::PriorityQueue> queue_;
    thread_pool::ThreadPool pool_;
};

}  // namespace dispatcher