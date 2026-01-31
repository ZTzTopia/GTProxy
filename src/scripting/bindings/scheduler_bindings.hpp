#pragma once
#include <sol/sol.hpp>

#include "../binding_module.hpp"
#include "../script_scheduler.hpp"

namespace scripting::bindings {
class SchedulerBindings final : public IBindingModule {
public:
    explicit SchedulerBindings(ScriptScheduler& scheduler)
        : scheduler_{ scheduler }
    {

    }

    [[nodiscard]] std::string_view name() const override { return "scheduler"; }

    void bind(sol::state& lua) override;

private:
    ScriptScheduler& scheduler_;
};
}
