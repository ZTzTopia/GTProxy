#pragma once
#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <sol/sol.hpp>

#include "lua_engine.hpp"
#include "../event/event.hpp"
#include "../network/client.hpp"
#include "../network/server.hpp"
#include "../packet/packet_helper.hpp"

namespace scripting {
struct LuaEventContext {
    event::Type type;
    const event::Event* event_ptr;
    std::vector<std::byte> raw_data;
    std::shared_ptr<bool> is_valid;
    std::shared_ptr<packet::IPacket> packet;
    std::optional<event::Direction> direction;

    void check_valid() const
    {
        if (!is_valid || !*is_valid) {
            throw std::runtime_error{ "Event context is no longer valid (event has finished processing)" };
        }
    }

    void cancel() const
    {
        check_valid();
        if (event_ptr) {
            event_ptr->cancel();
        }
    }

    [[nodiscard]] bool is_canceled() const
    {
        check_valid();
        return event_ptr ? event_ptr->canceled : false;
    }

    [[nodiscard]] std::string type_name() const
    {
        check_valid();
        if (event::is_packet_event(type)) {
            return std::string(magic_enum::enum_name(event::packet_id_from_type(type)));
        }
        return std::string(magic_enum::enum_name(type));
    }

    [[nodiscard]] bool has_packet() const
    {
        check_valid();
        return packet != nullptr;
    }

    [[nodiscard]] bool is_raw() const
    {
        check_valid();
        return !raw_data.empty() && !packet;
    }

    [[nodiscard]] sol::table get_data(sol::state& lua) const
    {
        check_valid();
        sol::table data = lua.create_table();
        for (size_t i = 0; i < raw_data.size(); ++i) {
            data[i + 1] = static_cast<int>(raw_data[i]);
        }
        return data;
    }

    [[nodiscard]] sol::object parse_packet(sol::this_state s);
};

class ScriptEventBridge : public utils::types::Immobile {
public:
    explicit ScriptEventBridge(
        event::Dispatcher& dispatcher,
        LuaEngine& engine,
        network::Client& client,
        network::Server& server
    );

    ~ScriptEventBridge();

    std::size_t register_callback(
        const std::string& event_name, 
        sol::protected_function callback,
        int8_t priority = event::Priority::Normal
    );
    bool unregister_callback(std::size_t handle);

    [[nodiscard]] LuaEngine& engine() const { return engine_; }
    [[nodiscard]] network::Client& client() const { return client_; }
    [[nodiscard]] network::Server& server() const { return server_; }

    void register_event_context_type();

private:
    void setup_event_listeners();
    void invoke_callbacks(event::Type type, const event::Event& event) const;
    static std::optional<event::Type> string_to_event_type(const std::string& name);

private:
    struct CallbackEntry {
        std::size_t handle;
        event::Type type;
        int8_t priority;
        sol::protected_function callback;
    };

    event::Dispatcher& dispatcher_;
    LuaEngine& engine_;
    network::Client& client_;
    network::Server& server_;

    std::vector<std::shared_ptr<CallbackEntry>> callbacks_;
    std::unordered_map<event::Type, event::Dispatcher::Handle> event_handles_;
    std::size_t next_handle_;
};
}
