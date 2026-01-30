#pragma once
#define SOL_ALL_SAFETIES_ON 1
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

    void bind(sol::state& lua) override
    {
        auto event_table{ lua.create_table() };

        auto normalize_event_name = [](const std::string& event_name) -> std::string {
            if (event_name.starts_with("server:")) {
                return "Server" + event_name.substr(event_name.find(":") + 1);
            }

            if (event_name.starts_with("client:")) {
                return "Client" + event_name.substr(event_name.find(":") + 1);
            }

            return event_name;
        };

        event_table.set_function("on", sol::overload(
            [this, &normalize_event_name](const std::string& event_name, sol::protected_function callback) {
                return bridge_.register_callback(normalize_event_name(event_name), std::move(callback));
            },
            [this, &normalize_event_name](const std::string& event_name, sol::protected_function callback, const int priority) {
                return bridge_.register_callback(
                    normalize_event_name(event_name),
                    std::move(callback),
                    static_cast<int8_t>(priority)
                );
            }
        ));

        event_table.set_function("off", [this](const std::size_t handle) {
            return bridge_.unregister_callback(handle);
        });

        lua["event"] = event_table;
    }

private:
    ScriptEventBridge& bridge_;
};
}
