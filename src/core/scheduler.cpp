#include "scheduler.hpp"

#include <algorithm>
#include <spdlog/spdlog.h>

namespace core {
Scheduler::Scheduler(std::size_t num_threads)
    : running_{ true }
    , paused_{ false }
    , next_id_{ 1 }
    , active_tasks_{ 0 }
{
    if (num_threads == 0) {
        num_threads = 1;
    }

    workers_.reserve(num_threads);
    for (std::size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back(&Scheduler::worker_thread, this);
    }

    timer_thread_ = std::thread(&Scheduler::timer_thread, this);

    spdlog::debug("Scheduler started with {} worker threads", num_threads);
}

Scheduler::~Scheduler()
{
    stop();
}

void Scheduler::stop()
{
    {
        std::lock_guard lock(mutex_);
        if (!running_) {
            return;
        }
        running_ = false;
    }

    timer_cv_.notify_all();
    worker_cv_.notify_all();

    if (timer_thread_.joinable()) {
        timer_thread_.join();
    }

    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    spdlog::debug("Scheduler stopped");
}

TaskId Scheduler::generate_id()
{
    return next_id_.fetch_add(1, std::memory_order_relaxed);
}

TaskId Scheduler::schedule(std::function<void()> callback, const TaskOptions& options)
{
    if (!callback) {
        return INVALID_TASK_ID;
    }

    const TaskId id = generate_id(); {
        Task task{
            .id = id,
            .tag = options.tag,
            .priority = options.priority,
            .execute_at = std::chrono::steady_clock::now() + options.delay,
            .interval = options.interval,
            .callback = std::move(callback)
        };
        std::lock_guard lock(mutex_);

        if (!running_) {
            return INVALID_TASK_ID;
        }

        pending_tasks_.insert(id);

        if (!options.tag.empty()) {
            tasks_by_tag_[options.tag].insert(id);
        }

        task_queue_.push(std::move(task));
    }

    timer_cv_.notify_one();

    return id;
}

TaskId Scheduler::schedule_delayed(
    std::function<void()> callback,
    std::chrono::milliseconds delay,
    const std::string& tag,
    const TaskPriority priority
) {
    return schedule(std::move(callback), TaskOptions{
        .tag = tag,
        .priority = priority,
        .delay = delay,
        .interval = std::chrono::milliseconds{ 0 }
    });
}

TaskId Scheduler::schedule_periodic(
    std::function<void()> callback,
    std::chrono::milliseconds interval,
    const std::string& tag,
    TaskPriority priority,
    std::chrono::milliseconds initial_delay
) {
    return schedule(std::move(callback), TaskOptions{
        .tag = tag,
        .priority = priority,
        .delay = initial_delay,
        .interval = interval
    });
}

TaskId Scheduler::schedule_immediate(
    std::function<void()> callback,
    const std::string& tag,
    TaskPriority priority
) {
    return schedule(std::move(callback), TaskOptions{
        .tag = tag,
        .priority = priority,
        .delay = std::chrono::milliseconds{ 0 },
        .interval = std::chrono::milliseconds{ 0 }
    });
}

bool Scheduler::cancel(TaskId id)
{
    if (id == INVALID_TASK_ID) {
        return false;
    }

    std::lock_guard lock(mutex_);

    if (!pending_tasks_.contains(id)) {
        return false;
    }

    cancelled_tasks_.insert(id);
    return true;
}

std::size_t Scheduler::cancel_by_tag(const std::string& tag)
{
    if (tag.empty()) {
        return 0;
    }

    std::lock_guard lock(mutex_);

    const auto it{ tasks_by_tag_.find(tag) };
    if (it == tasks_by_tag_.end()) {
        return 0;
    }

    std::size_t count = 0;
    for (TaskId id : it->second) {
        if (pending_tasks_.contains(id)) {
            cancelled_tasks_.insert(id);
            ++count;
        }
    }

    return count;
}

void Scheduler::cancel_all()
{
    std::lock_guard lock(mutex_);

    for (TaskId id : pending_tasks_) {
        cancelled_tasks_.insert(id);
    }
}

bool Scheduler::is_pending(TaskId id) const
{
    std::lock_guard lock(mutex_);
    return pending_tasks_.contains(id) && !cancelled_tasks_.contains(id);
}

bool Scheduler::is_running(TaskId id) const
{
    std::lock_guard lock(mutex_);
    return running_tasks_.contains(id);
}

std::size_t Scheduler::pending_count() const
{
    std::lock_guard lock(mutex_);
    std::size_t count{ 0 };
    for (TaskId id : pending_tasks_) {
        if (cancelled_tasks_.find(id) == cancelled_tasks_.end()) {
            ++count;
        }
    }

    return count;
}

std::size_t Scheduler::pending_count_by_tag(const std::string& tag) const
{
    if (tag.empty()) {
        return 0;
    }

    std::lock_guard lock(mutex_);

    const auto it{ tasks_by_tag_.find(tag) };
    if (it == tasks_by_tag_.end()) {
        return 0;
    }

    std::size_t count = 0;
    for (TaskId id : it->second) {
        if (pending_tasks_.contains(id) && !cancelled_tasks_.contains(id)) {
            ++count;
        }
    }
    return count;
}

void Scheduler::pause()
{
    paused_ = true;
}

void Scheduler::resume()
{
    paused_ = false;
    timer_cv_.notify_all();
    worker_cv_.notify_all();
}

void Scheduler::wait_all()
{
    std::unique_lock lock(mutex_);
    idle_cv_.wait(lock, [this] {
        return pending_tasks_.empty() && running_tasks_.empty();
    });
}

void Scheduler::timer_thread()
{
    while (running_) {
        std::unique_lock lock(mutex_);

        if (task_queue_.empty()) {
            timer_cv_.wait(lock, [this] {
                return !running_ || !task_queue_.empty();
            });

            if (!running_) {
                break;
            }
        }

        if (paused_) {
            timer_cv_.wait(lock, [this] {
                return !running_ || !paused_;
            });

            if (!running_) {
                break;
            }
            continue;
        }

        if (task_queue_.empty()) {
            continue;
        }

        const auto& top_task = task_queue_.top();

        if (auto now = std::chrono::steady_clock::now(); top_task.execute_at <= now) {
            Task task = std::move(const_cast<Task&>(task_queue_.top()));
            task_queue_.pop();

            if (cancelled_tasks_.contains(task.id)) {
                pending_tasks_.erase(task.id);
                cancelled_tasks_.erase(task.id);

                if (!task.tag.empty()) {
                    auto tag_it = tasks_by_tag_.find(task.tag);
                    if (tag_it != tasks_by_tag_.end()) {
                        tag_it->second.erase(task.id);
                        if (tag_it->second.empty()) {
                            tasks_by_tag_.erase(tag_it);
                        }
                    }
                }
                continue;
            }

            ready_queue_.push(std::move(task));
            worker_cv_.notify_one();
        }
        else {
            auto wait_duration = top_task.execute_at - now;
            timer_cv_.wait_for(lock, wait_duration, [this] {
                return !running_ || paused_;
            });
        }
    }
}

void Scheduler::worker_thread()
{
    while (running_) {
        Task task;

        {
            std::unique_lock lock(mutex_);

            worker_cv_.wait(lock, [this] {
                return !running_ || !ready_queue_.empty();
            });

            if (!running_ && ready_queue_.empty()) {
                break;
            }

            if (ready_queue_.empty()) {
                continue;
            }

            task = std::move(ready_queue_.front());
            ready_queue_.pop();

            pending_tasks_.erase(task.id);
            running_tasks_.insert(task.id);
            ++active_tasks_;
        }

        try {
            task.callback();
        }
        catch (const std::exception& e) {
            spdlog::error("Scheduler task {} threw exception: {}", task.id, e.what());
        }
        catch (...) {
            spdlog::error("Scheduler task {} threw unknown exception", task.id);
        }

        {
            std::lock_guard lock(mutex_);

            running_tasks_.erase(task.id);
            --active_tasks_;

            if (!task.tag.empty()) {
                if (auto tag_it = tasks_by_tag_.find(task.tag); tag_it != tasks_by_tag_.end()) {
                    tag_it->second.erase(task.id);
                    if (tag_it->second.empty()) {
                        tasks_by_tag_.erase(tag_it);
                    }
                }
            }

            if (task.interval.count() > 0 && !cancelled_tasks_.contains(task.id)) {
                TaskId new_id = generate_id();

                Task new_task{
                    .id = new_id,
                    .tag = task.tag,
                    .priority = task.priority,
                    .execute_at = std::chrono::steady_clock::now() + task.interval,
                    .interval = task.interval,
                    .callback = std::move(task.callback)
                };

                pending_tasks_.insert(new_id);
                if (!new_task.tag.empty()) {
                    tasks_by_tag_[new_task.tag].insert(new_id);
                }

                task_queue_.push(std::move(new_task));
                timer_cv_.notify_one();
            }

            cancelled_tasks_.erase(task.id);

            if (pending_tasks_.empty() && running_tasks_.empty()) {
                idle_cv_.notify_all();
            }
        }
    }
}
}
