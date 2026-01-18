#pragma once
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace core {
using TaskId = std::uint64_t;
constexpr TaskId INVALID_TASK_ID = 0;

enum class TaskPriority : std::int8_t {
    Highest = -128,
    High = -64,
    Normal = 0,
    Low = 64,
    Lowest = 127
};

struct TaskOptions {
    std::string tag;
    TaskPriority priority = TaskPriority::Normal;
    std::chrono::milliseconds delay{ 0 };
    std::chrono::milliseconds interval{ 0 };
};

class Scheduler {
public:
    explicit Scheduler(std::size_t num_threads = std::thread::hardware_concurrency());
    ~Scheduler();

    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;
    Scheduler(Scheduler&&) = delete;
    Scheduler& operator=(Scheduler&&) = delete;

    TaskId schedule(std::function<void()> callback, const TaskOptions& options = {});
    TaskId schedule_delayed(
        std::function<void()> callback,
        std::chrono::milliseconds delay,
        const std::string& tag = "",
        TaskPriority priority = TaskPriority::Normal
    );
    TaskId schedule_periodic(
        std::function<void()> callback,
        std::chrono::milliseconds interval,
        const std::string& tag = "",
        TaskPriority priority = TaskPriority::Normal,
        std::chrono::milliseconds initial_delay = std::chrono::milliseconds{ 0 }
    );
    TaskId schedule_immediate(
        std::function<void()> callback,
        const std::string& tag = "",
        TaskPriority priority = TaskPriority::Normal
    );

    bool cancel(TaskId id);
    std::size_t cancel_by_tag(const std::string& tag);
    void cancel_all();

    [[nodiscard]] bool is_pending(TaskId id) const;
    [[nodiscard]] bool is_running(TaskId id) const;
    [[nodiscard]] std::size_t pending_count() const;
    [[nodiscard]] std::size_t pending_count_by_tag(const std::string& tag) const;

    void pause();
    void resume();
    void stop();

    void wait_all();

private:
    struct Task {
        TaskId id;
        std::string tag;
        TaskPriority priority;
        std::chrono::steady_clock::time_point execute_at;
        std::chrono::milliseconds interval;
        std::function<void()> callback;

        // For priority queue comparison (min-heap by time, then by priority)
        bool operator>(const Task& other) const
        {
            if (execute_at != other.execute_at) {
                return execute_at > other.execute_at;
            }

            return static_cast<std::int8_t>(priority) > static_cast<std::int8_t>(other.priority);
        }
    };

    void worker_thread();
    void timer_thread();
    TaskId generate_id();

private:
    std::priority_queue<Task, std::vector<Task>, std::greater<Task>> task_queue_;
    std::unordered_set<TaskId> cancelled_tasks_;
    std::unordered_set<TaskId> pending_tasks_;
    std::unordered_set<TaskId> running_tasks_;
    std::unordered_map<std::string, std::unordered_set<TaskId>> tasks_by_tag_;

    std::queue<Task> ready_queue_;

    mutable std::mutex mutex_;
    std::condition_variable timer_cv_;
    std::condition_variable worker_cv_;

    std::vector<std::thread> workers_;
    std::thread timer_thread_;

    std::atomic<bool> running_;
    std::atomic<bool> paused_;
    std::atomic<TaskId> next_id_;
    std::atomic<std::size_t> active_tasks_;

    std::condition_variable idle_cv_;
};
}
