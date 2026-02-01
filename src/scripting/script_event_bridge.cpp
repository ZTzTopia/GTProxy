#include "script_event_bridge.hpp"
#include "../packet/generic_packets.hpp"
#include "../packet/packet_types.hpp"
#include "../packet/packet_variant.hpp"
#include "../packet/game/player.hpp"
#include "../packet/game/server.hpp"
#include "../packet/game/world.hpp"
#include "../packet/message/chat.hpp"
#include "../packet/message/exit.hpp"
#include "../packet/message/input.hpp"
#include "../packet/message/server_hello.hpp"
#include "../utils/byte_stream.hpp"
#include "../utils/text_parse.hpp"

#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

namespace scripting {

namespace {
const std::unordered_map<std::string, packet::PacketId> PACKET_ID_MAP = {
    {"ServerHello", packet::PacketId::ServerHello},
    {"Quit", packet::PacketId::Quit},
    {"QuitToExit", packet::PacketId::QuitToExit},
    {"JoinRequest", packet::PacketId::JoinRequest},
    {"ValidateWorld", packet::PacketId::ValidateWorld},
    {"Input", packet::PacketId::Input},
    {"Log", packet::PacketId::Log},
    {"Disconnect", packet::PacketId::Disconnect},
    {"OnSendToServer", packet::PacketId::OnSendToServer},
    {"OnNameChanged", packet::PacketId::OnNameChanged},
    {"OnChangeSkin", packet::PacketId::OnChangeSkin},
    {"OnSpawn", packet::PacketId::OnSpawn},
    {"OnRemove", packet::PacketId::OnRemove},
};
}

sol::object LuaEventContext::parse_packet(sol::this_state s)
{
    check_valid();
    if (raw_data.empty()) {
        spdlog::debug("[parse_packet] raw_data is empty, returning nil");
        return sol::make_object(s, sol::lua_nil);
    }

    ByteStream stream{ raw_data.data(), raw_data.size() };
    std::uint32_t type_val{ 0 };
    if (!stream.read(type_val)) {
        spdlog::debug("[parse_packet] Failed to read type_val, returning nil");
        return sol::make_object(s, sol::lua_nil);
    }

    const auto msg_type = static_cast<packet::NetMessageType>(type_val);
    spdlog::debug("[parse_packet] msg_type={} (raw={})", static_cast<int>(msg_type), type_val);

    if (msg_type == packet::NET_MESSAGE_GAME_MESSAGE || 
        msg_type == packet::NET_MESSAGE_GENERIC_TEXT ||
        msg_type == packet::NET_MESSAGE_ERROR) 
    {
        const auto remaining = stream.get_size() - stream.get_read_offset();
        if (remaining > 0) {
            std::string content(reinterpret_cast<const char*>(stream.get_raw_ptr() + stream.get_read_offset()), remaining);
            if (!content.empty() && content.back() == '\0') {
                content.pop_back();
            }
            spdlog::debug("[parse_packet] Returning TextParse with {} bytes", content.size());
            return sol::make_object(s, TextParse{ content });
        }
    }
    else if (msg_type == packet::NET_MESSAGE_GAME_PACKET) {
        packet::GameUpdatePacket game_packet{};
        if (stream.read(game_packet)) {
            if (game_packet.flags.extended && game_packet.data_size > 0) {
                std::vector<std::byte> extra_data(game_packet.data_size);
                
                if (stream.read_data(extra_data.data(), game_packet.data_size)) {
                    packet::PacketVariant var;
                    if (var.deserialize(extra_data)) {
                        spdlog::debug("[parse_packet] Returning PacketVariant");
                        return sol::make_object(s, std::move(var));
                    }
                }
            }

            spdlog::debug("[parse_packet] Returning GameUpdatePacket");
            return sol::make_object(s, game_packet);
        }
    }

    spdlog::debug("[parse_packet] Unhandled msg_type={}, returning nil", static_cast<int>(msg_type));
    return sol::make_object(s, sol::lua_nil);
}

ScriptEventBridge::ScriptEventBridge(
    event::Dispatcher& dispatcher,
    LuaEngine& engine,
    network::Client& client,
    network::Server& server
)
    : dispatcher_{ dispatcher }
    , engine_{ engine }
    , client_{ client }
    , server_{ server }
    , next_handle_{ 0 }
{
    register_event_context_type();
    setup_event_listeners();
    spdlog::info("Script event bridge initialized");
}

ScriptEventBridge::~ScriptEventBridge()
{
    for (auto& [type, handle] : event_handles_) {
        dispatcher_.removeListener(type, handle);
    }
}

void ScriptEventBridge::register_event_context_type()
{
    sol::state& lua{ engine_.state() };
    lua.new_usertype<LuaEventContext>("EventContext",
        sol::no_constructor,
        "cancel", &LuaEventContext::cancel,
        "is_canceled", &LuaEventContext::is_canceled,
        "type", sol::property(&LuaEventContext::type_name),
        "has_packet", &LuaEventContext::has_packet,
        "is_raw", &LuaEventContext::is_raw,
        "get_data", [&lua](const LuaEventContext& ctx) {
            return ctx.get_data(lua);
        },
        "get_packet", [](const LuaEventContext& ctx, sol::this_state s) -> sol::object {
            ctx.check_valid();
            if (!ctx.packet) {
                return sol::make_object(s, sol::lua_nil);
            }

            const auto pid = ctx.packet->id();
            switch (pid) {
                case packet::PacketId::Input:
                    return sol::make_object(s, static_cast<packet::message::Input*>(ctx.packet.get()));
                case packet::PacketId::Log:
                    return sol::make_object(s, static_cast<packet::message::Log*>(ctx.packet.get()));
                case packet::PacketId::JoinRequest:
                    return sol::make_object(s, static_cast<packet::message::JoinRequest*>(ctx.packet.get()));
                case packet::PacketId::Quit:
                    return sol::make_object(s, static_cast<packet::message::Quit*>(ctx.packet.get()));
                case packet::PacketId::QuitToExit:
                    return sol::make_object(s, static_cast<packet::message::QuitToExit*>(ctx.packet.get()));
                case packet::PacketId::ServerHello:
                    return sol::make_object(s, static_cast<packet::message::ServerHello*>(ctx.packet.get()));
                case packet::PacketId::OnSendToServer:
                    return sol::make_object(s, static_cast<packet::game::OnSendToServer*>(ctx.packet.get()));
                case packet::PacketId::OnNameChanged:
                    return sol::make_object(s, static_cast<packet::game::OnNameChanged*>(ctx.packet.get()));
                case packet::PacketId::OnChangeSkin:
                    return sol::make_object(s, static_cast<packet::game::OnChangeSkin*>(ctx.packet.get()));
                case packet::PacketId::OnSpawn:
                    return sol::make_object(s, static_cast<packet::game::OnSpawn*>(ctx.packet.get()));
                case packet::PacketId::OnRemove:
                    return sol::make_object(s, static_cast<packet::game::OnRemove*>(ctx.packet.get()));
                case packet::PacketId::Unknown:
                default:
                    // Try to cast to generic packet types
                    if (auto* text_pkt = dynamic_cast<packet::GenericTextPacket*>(ctx.packet.get())) {
                        return sol::make_object(s, text_pkt);
                    }
                    if (auto* var_pkt = dynamic_cast<packet::GenericVariantPacket*>(ctx.packet.get())) {
                        return sol::make_object(s, var_pkt);
                    }
                    if (auto* game_pkt = dynamic_cast<packet::GenericGamePacket*>(ctx.packet.get())) {
                        return sol::make_object(s, game_pkt);
                    }
                    return sol::make_object(s, ctx.packet.get());
            }
        },
        "parse", &LuaEventContext::parse_packet,
        "packet_id", [](const LuaEventContext& ctx) -> std::string {
            ctx.check_valid();
            if (ctx.packet) {
                return std::string(magic_enum::enum_name(ctx.packet->id()));
            }
            return "Unknown";
        },
        "direction", [](const LuaEventContext& ctx) -> std::string {
            ctx.check_valid();
            if (ctx.direction) {
                return *ctx.direction == event::Direction::ClientBound ? "ClientBound" : "ServerBound";
            }
            return "Unknown";
        }
    );
}

void ScriptEventBridge::setup_event_listeners()
{
    auto make_handler = [this](event::Type type) {
        return [this, type](const event::Event& e) {
            invoke_callbacks(type, e);
        };
    };

    constexpr auto event_values = magic_enum::enum_values<event::Type>();
    for (const auto type : event_values) {
        if (type == event::Type::Max) continue;

        event_handles_[type] = dispatcher_.appendListener(
            type,
            make_handler(type),
            event::Priority::FairlyHigh
        );
    }

    for (const auto& [name, pid] : PACKET_ID_MAP) {
        const auto type = event::packet_event_type(pid);
        event_handles_[type] = dispatcher_.appendListener(
            type,
            make_handler(type),
            event::Priority::FairlyHigh
        );
        spdlog::debug("Registered packet event listener for '{}' (type={})", name, static_cast<uint32_t>(type));
    }
}

namespace {
template<packet::PacketId P>
bool fill_typed_context(const event::Event& e, scripting::LuaEventContext& ctx)
{
    if (const auto* typed_evt = dynamic_cast<const event::TypedPacketEvent<P>*>(&e)) {
        ctx.packet = typed_evt->packet;
        ctx.direction = typed_evt->direction;
        return true;
    }
    return false;
}

void try_fill_typed_context(packet::PacketId pid, const event::Event& event, scripting::LuaEventContext& ctx)
{
    switch (pid) {
    case packet::PacketId::OnSendToServer: fill_typed_context<packet::PacketId::OnSendToServer>(event, ctx); break;
    case packet::PacketId::Quit: fill_typed_context<packet::PacketId::Quit>(event, ctx); break;
    case packet::PacketId::QuitToExit: fill_typed_context<packet::PacketId::QuitToExit>(event, ctx); break;
    case packet::PacketId::JoinRequest: fill_typed_context<packet::PacketId::JoinRequest>(event, ctx); break;
    case packet::PacketId::ValidateWorld: fill_typed_context<packet::PacketId::ValidateWorld>(event, ctx); break;
    case packet::PacketId::Input: fill_typed_context<packet::PacketId::Input>(event, ctx); break;
    case packet::PacketId::Log: fill_typed_context<packet::PacketId::Log>(event, ctx); break;
    case packet::PacketId::Disconnect: fill_typed_context<packet::PacketId::Disconnect>(event, ctx); break;
    case packet::PacketId::ServerHello: fill_typed_context<packet::PacketId::ServerHello>(event, ctx); break;
    case packet::PacketId::OnNameChanged: fill_typed_context<packet::PacketId::OnNameChanged>(event, ctx); break;
    case packet::PacketId::OnChangeSkin: fill_typed_context<packet::PacketId::OnChangeSkin>(event, ctx); break;
    case packet::PacketId::OnSpawn: fill_typed_context<packet::PacketId::OnSpawn>(event, ctx); break;
    case packet::PacketId::OnRemove: fill_typed_context<packet::PacketId::OnRemove>(event, ctx); break;
    default: break;
    }
}
}

void ScriptEventBridge::invoke_callbacks(event::Type type, const event::Event& event) const
{
    if (event.canceled) {
        return;
    }

    LuaEventContext ctx;
    ctx.type = type;
    ctx.event_ptr = &event;
    ctx.packet = nullptr;
    ctx.is_valid = std::make_shared<bool>(true);

    if (const auto* raw_event = dynamic_cast<const event::RawPacketEvent*>(&event)) {
        ctx.raw_data.assign(raw_event->data.begin(), raw_event->data.end());
    }

    if (const auto* pkt_event = dynamic_cast<const event::PacketEvent*>(&event)) {
        ctx.packet = pkt_event->packet;
    }

    if (event::is_packet_event(type)) {
        const auto pid = event::packet_id_from_type(type);
        try_fill_typed_context(pid, event, ctx);
    }

    std::vector<std::shared_ptr<CallbackEntry>> applicable_callbacks;
    applicable_callbacks.reserve(callbacks_.size());

    for (const auto& entry : callbacks_) {
        if (entry->type == type) {
            applicable_callbacks.push_back(entry);
        }
    }

    std::ranges::sort(applicable_callbacks, [](const auto& a, const auto& b) {
        return a->priority < b->priority;
    });

    for (const auto& entry : applicable_callbacks) {
        if (event.canceled) {
            *ctx.is_valid = false;
            return;
        }

        sol::protected_function_result result = entry->callback(ctx);

        if (!result.valid()) {
            sol::error err = result;
            spdlog::error("Lua callback error: {}", err.what());
            continue;
        }

        if (result.get_type() == sol::type::boolean) {
            if (const bool should_continue{ result.get<bool>() }; !should_continue) {
                spdlog::debug("Lua callback canceled event: {}", magic_enum::enum_name(type));
                event.cancel();
                *ctx.is_valid = false;
                return;
            }
        }
    }

    *ctx.is_valid = false;
}

std::size_t ScriptEventBridge::register_callback(
    const std::string& event_name, 
    sol::safe_function callback,
    const int8_t priority
)
{
    const auto type_opt{ string_to_event_type(event_name) };
    
    if (!type_opt.has_value()) {
        spdlog::error("Unknown event type '{}', registration rejected", event_name);
        throw std::runtime_error{ "Unknown event type: " + event_name };
    }

    const event::Type type{ type_opt.value() };
    std::size_t handle = next_handle_++;
    callbacks_.push_back(std::make_shared<CallbackEntry>(handle, type, priority, std::move(callback)));

    spdlog::debug(
        "Registered Lua callback for event '{}' with handle {} (priority {})",
        event_name,
        handle,
        static_cast<int>(priority)
    );
    return handle;
}

bool ScriptEventBridge::unregister_callback(std::size_t handle)
{
    const auto it = std::ranges::find_if(callbacks_,
        [handle](const std::shared_ptr<CallbackEntry>& entry) { return entry->handle == handle; });

    if (it != callbacks_.end()) {
        callbacks_.erase(it);
        spdlog::debug("Unregistered Lua callback with handle {}", handle);
        return true;
    }

    return false;
}

std::optional<event::Type> ScriptEventBridge::string_to_event_type(const std::string& name)
{
    if (const auto type{ magic_enum::enum_cast<event::Type>(name) }; type.has_value()) {
        return type.value();
    }

    if (const auto it = PACKET_ID_MAP.find(name); it != PACKET_ID_MAP.end()) {
        return event::packet_event_type(it->second);
    }

    return std::nullopt;
}
}
