#pragma once
#include <sol/sol.hpp>

#include "../binding_module.hpp"
#include "../script_event_bridge.hpp"

namespace scripting::bindings {
class EventBindings final : public IBindingModule {
public:
    explicit EventBindings(ScriptEventBridge& bridge)
        : bridge_{ bridge }
    {

    }

    [[nodiscard]] std::string_view name() const override { return "event"; }

    void bind(sol::state& lua) override;

private:
    ScriptEventBridge& bridge_;
};
}
