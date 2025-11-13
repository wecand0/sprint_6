#include "task_dispatcher.hpp"

namespace dispatcher {

TaskDispatcher::TaskDispatcher(const size_t thread_count,
                               const std::flat_map<TaskPriority, queue::QueueOptions> &tasks_options)
    : queue_{std::make_shared<queue::PriorityQueue>(tasks_options)}, pool_{queue_, thread_count} {}

TaskDispatcher::~TaskDispatcher() = default;

void TaskDispatcher::schedule(TaskPriority priority, std::function<void()> task) const {
    queue_->push(priority, std::move(task));
}

}  // namespace dispatcher