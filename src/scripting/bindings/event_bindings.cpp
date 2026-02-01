#include "event_bindings.hpp"

namespace scripting::bindings {
void EventBindings::bind(sol::state& lua)
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
}
