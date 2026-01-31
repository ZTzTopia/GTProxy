#pragma once
#include <chrono>
#include <memory>
#include <vector>

#include <sol/sol.hpp>

#include "lua_engine.hpp"
#include "../utils/types.hpp"

namespace scripting {
using TaskId = std::uint64_t;
constexpr TaskId INVALID_TASK_ID = 0;

class ScriptScheduler final : public utils::types::Immobile {
public:
    explicit ScriptScheduler(LuaEngine& engine);
    ~ScriptScheduler() = default;

    TaskId schedule(
        std::chrono::milliseconds delay,
        sol::protected_function callback
    );

    TaskId schedule_periodic(
        std::chrono::milliseconds interval,
        sol::protected_function callback,
        std::chrono::milliseconds initial_delay = std::chrono::milliseconds{ 0 }
    );

    bool cancel(TaskId id);
    void cancel_all();

    void update(std::chrono::microseconds elapsed);

    [[nodiscard]] bool is_pending(TaskId id) const;
    [[nodiscard]] std::size_t pending_count() const;

    [[nodiscard]] LuaEngine& engine() const { return engine_; }

private:
    struct Task {
        TaskId id;
        std::chrono::microseconds remaining;
        std::chrono::microseconds interval;
        sol::protected_function callback;
        bool periodic;
        bool cancelled;
    };

    TaskId generate_id();

private:
    LuaEngine& engine_;
    std::vector<std::shared_ptr<Task>> tasks_;
    TaskId next_id_;
};
}
