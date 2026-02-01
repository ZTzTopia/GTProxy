#include "scheduler_bindings.hpp"

namespace scripting::bindings {
void SchedulerBindings::bind(sol::state& lua)
{
    auto scheduler_table{ lua.create_table() };

    scheduler_table.set_function("schedule", sol::overload(
        [this](const int delay_ms, sol::protected_function callback) {
                return scheduler_.schedule(
                    std::chrono::milliseconds{ delay_ms },
                    std::move(callback)
                );
            },
            [this](const double delay_ms, sol::protected_function callback) {
                return scheduler_.schedule(
                    std::chrono::milliseconds{ static_cast<int>(delay_ms) },
                    std::move(callback)
                );
            }
    ));

    scheduler_table.set_function("schedule_periodic", sol::overload(
        [this](const int interval_ms, sol::protected_function callback) {
                return scheduler_.schedule_periodic(
                    std::chrono::milliseconds{ interval_ms },
                    std::move(callback)
                );
            },
            [this](const int interval_ms, sol::protected_function callback, const int initial_delay_ms) {
                return scheduler_.schedule_periodic(
                    std::chrono::milliseconds{ interval_ms },
                    std::move(callback),
                    std::chrono::milliseconds{ initial_delay_ms }
                );
            },
            [this](const double interval_ms, sol::protected_function callback) {
                return scheduler_.schedule_periodic(
                    std::chrono::milliseconds{ static_cast<int>(interval_ms) },
                    std::move(callback)
                );
            },
            [this](const double interval_ms, sol::protected_function callback, const double initial_delay_ms) {
                return scheduler_.schedule_periodic(
                    std::chrono::milliseconds{ static_cast<int>(interval_ms) },
                    std::move(callback),
                    std::chrono::milliseconds{ static_cast<int>(initial_delay_ms) }
                );
            }
    ));

    scheduler_table.set_function("cancel", [this](TaskId id) {
        return scheduler_.cancel(id);
    });

    scheduler_table.set_function("cancel_all", [this]() {
        scheduler_.cancel_all();
    });

    scheduler_table.set_function("is_pending", [this](TaskId id) {
        return scheduler_.is_pending(id);
    });

    scheduler_table.set_function("pending_count", [this]() {
        return scheduler_.pending_count();
    });

    lua["scheduler"] = scheduler_table;
}
}
