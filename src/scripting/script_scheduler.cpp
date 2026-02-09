#include "script_scheduler.hpp"

#include <algorithm>
#include <spdlog/spdlog.h>

namespace scripting {
ScriptScheduler::ScriptScheduler(LuaEngine& engine)
    : engine_{ engine }
    , next_id_{ 1 }
{
    spdlog::info("Script scheduler initialized");
}

TaskId ScriptScheduler::generate_id()
{
    return next_id_++;
}

TaskId ScriptScheduler::schedule(
    std::chrono::milliseconds delay,
    sol::protected_function callback
) {
    const auto id{ generate_id() };
    auto task{ std::make_shared<Task>() };
    task->id = id;
    task->remaining = std::chrono::duration_cast<std::chrono::microseconds>(delay);
    task->interval = std::chrono::microseconds{ 0 };
    task->callback = std::move(callback);
    task->periodic = false;
    task->cancelled = false;

    tasks_.push_back(std::move(task));

    spdlog::debug("Scheduled one-shot task {} with delay {}ms", id, delay.count());
    return id;
}

TaskId ScriptScheduler::schedule_periodic(
    std::chrono::milliseconds interval,
    sol::protected_function callback,
    std::chrono::milliseconds initial_delay
) {
    const auto id{ generate_id() };
    auto task{ std::make_shared<Task>() };
    task->id = id;
    task->remaining = std::chrono::duration_cast<std::chrono::microseconds>(
        initial_delay.count() > 0 ? initial_delay : interval
    );
    task->interval = std::chrono::duration_cast<std::chrono::microseconds>(interval);
    task->callback = std::move(callback);
    task->periodic = true;
    task->cancelled = false;

    tasks_.push_back(std::move(task));

    spdlog::debug("Scheduled periodic task {} with interval {}ms", id, interval.count());
    return id;
}

bool ScriptScheduler::cancel(TaskId id)
{
    const auto it{ std::ranges::find_if(tasks_,
        [id](const std::shared_ptr<Task>& task) { return task->id == id; }) };

    if (it != tasks_.end()) {
        (*it)->cancelled = true;
        spdlog::debug("Cancelled task {}", id);
        return true;
    }

    return false;
}

void ScriptScheduler::cancel_all()
{
    for (auto& task : tasks_) {
        task->cancelled = true;
    }

    spdlog::debug("Cancelled all {} tasks", tasks_.size());
}

void ScriptScheduler::update(std::chrono::microseconds elapsed)
{
    if (tasks_.empty()) {
        return;
    }

    std::vector<std::shared_ptr<Task>> tasks_to_execute;

    for (auto& task : tasks_) {
        if (task->cancelled) {
            continue;
        }

        task->remaining -= elapsed;

        if (task->remaining <= std::chrono::microseconds::zero()) {
            tasks_to_execute.push_back(task);

            if (task->periodic) {
                task->remaining = task->interval;
            }
        }
    }

    for (const auto& task : tasks_to_execute) {
        if (task->cancelled) {
            continue;
        }

        sol::protected_function_result result{ task->callback() };

        if (!result.valid()) {
            sol::error err = result;
            spdlog::error("Scheduler callback error (task {}): {}", task->id, err.what());
        }
        else if (result.get_type() == sol::type::boolean) {
            if (const bool should_continue{ result.get<bool>() }; !should_continue && task->periodic) {
                task->cancelled = true;
                spdlog::debug("Periodic task {} stopped by callback returning false", task->id);
            }
        }

        if (!task->periodic) {
            task->cancelled = true;
        }
    }

    std::erase_if(tasks_, [](const std::shared_ptr<Task>& task) {
        return task->cancelled;
    });
}

bool ScriptScheduler::is_pending(TaskId id) const
{
    return std::ranges::any_of(
        tasks_,
        [id](const std::shared_ptr<Task>& task) {
            return task->id == id && !task->cancelled;
        }
    );
}

std::size_t ScriptScheduler::pending_count() const
{
    return std::ranges::count_if(
        tasks_,
        [](const std::shared_ptr<Task>& task) { return !task->cancelled; }
    );
}
}
