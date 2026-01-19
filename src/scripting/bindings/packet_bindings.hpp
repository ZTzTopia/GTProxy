#pragma once
#include "../binding_module.hpp"
#include "../../network/client.hpp"
#include "../../network/server.hpp"
#include "../../packet/packet_helper.hpp"
#include "../../packet/message/chat.hpp"
#include "../../packet/message/exit.hpp"
#include "../../packet/message/input.hpp"
#include "../../packet/message/server_hello.hpp"
#include "../../packet/game/on_send_to_server.hpp"
#include "../../packet/game/player.hpp"
#include "../../utils/text_parse.hpp"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <spdlog/spdlog.h>

namespace scripting::bindings {
class PacketBindings final : public IBindingModule {
public:
    explicit PacketBindings(network::Client& client, network::Server& server)
        : client_{ client }
        , server_{ server }
    {

    }

    [[nodiscard]] std::string_view name() const override { return "packet"; }

    void bind(sol::state& lua) override
    {
        bind_text_parse(lua);
        bind_base_packet(lua);
        bind_message_packets(lua);
        bind_game_packets(lua);
        bind_send_functions(lua);
    }

private:
    void bind_text_parse(sol::state& lua)
    {
        lua.new_usertype<TextParse>("TextParse",
            sol::constructors<TextParse(), TextParse(const std::string&, const std::string&)>(),
            "get", sol::overload(
                [](const TextParse& tp, const std::string& key) {
                    return tp.get(key, 0);
                },
                [](const TextParse& tp, const std::string& key, int index) {
                    return tp.get(key, index);
                }
            ),
            "add", [](TextParse& tp, const std::string& key, const std::string& value) {
                tp.add(key, { value });
            },
            "set", [](TextParse& tp, const std::string& key, const std::string& value) {
                tp.set(key, { value });
            },
            "remove", &TextParse::remove,
            "contains", &TextParse::contains,
            "keys", [](const TextParse& tp, sol::this_state s) {
                sol::state_view lua{ s };
                sol::table keys = lua.create_table();
                int index = 1;
                for (const auto& [key, _] : tp.get_data()) {
                    keys[index++] = key;
                }
                return keys;
            },
            "empty", &TextParse::empty,
            "get_raw", sol::overload(
                [](const TextParse& tp) {
                    return tp.get_raw();
                },
                [](const TextParse& tp, const std::string& delimiter) {
                    return tp.get_raw(delimiter);
                }
            )
        );
    }

    void bind_base_packet(sol::state& lua)
    {
        lua.new_usertype<packet::IPacket>("IPacket",
            sol::no_constructor,
            "id", &packet::IPacket::id,
            "channel", &packet::IPacket::channel
        );
    }

    void bind_message_packets(sol::state& lua)
    {
        lua.new_usertype<packet::message::ServerHello>("ServerHelloPacket",
            sol::constructors<packet::message::ServerHello()>(),
            sol::base_classes, sol::bases<packet::IPacket>()
        );

        lua.new_usertype<packet::message::Log>("LogPacket",
            sol::constructors<packet::message::Log()>(),
            sol::base_classes, sol::bases<packet::IPacket>(),
            "msg", &packet::message::Log::msg
        );

        lua.new_usertype<packet::message::Quit>("QuitPacket",
            sol::constructors<packet::message::Quit()>(),
            sol::base_classes, sol::bases<packet::IPacket>()
        );

        lua.new_usertype<packet::message::QuitToExit>("QuitToExitPacket",
            sol::constructors<packet::message::QuitToExit()>(),
            sol::base_classes, sol::bases<packet::IPacket>()
        );

        lua.new_usertype<packet::message::JoinRequest>("JoinRequestPacket",
            sol::constructors<packet::message::JoinRequest()>(),
            sol::base_classes, sol::bases<packet::IPacket>(),
            "world_name", &packet::message::JoinRequest::world_name,
            "invited_world", &packet::message::JoinRequest::invited_world
        );

        lua.new_usertype<packet::message::ValidateWorld>("ValidateWorldPacket",
            sol::constructors<packet::message::ValidateWorld()>(),
            sol::base_classes, sol::bases<packet::IPacket>(),
            "world_name", &packet::message::ValidateWorld::world_name
        );

        lua.new_usertype<packet::message::Input>("InputPacket",
            sol::constructors<packet::message::Input()>(),
            sol::base_classes, sol::bases<packet::IPacket>(),
            "text", &packet::message::Input::text
        );
    }

    void bind_game_packets(sol::state& lua)
    {
        lua.new_usertype<packet::game::Disconnect>("DisconnectPacket",
            sol::constructors<packet::game::Disconnect()>(),
            sol::base_classes, sol::bases<packet::IPacket>()
        );

        lua.new_usertype<packet::game::OnSendToServer>("OnSendToServerPacket",
            sol::constructors<packet::game::OnSendToServer()>(),
            sol::base_classes, sol::bases<packet::IPacket>(),
            "port", &packet::game::OnSendToServer::port,
            "token", &packet::game::OnSendToServer::token,
            "user", &packet::game::OnSendToServer::user,
            "address", &packet::game::OnSendToServer::address,
            "door_id", &packet::game::OnSendToServer::door_id,
            "uuid_token", &packet::game::OnSendToServer::uuid_token,
            "login_mode", &packet::game::OnSendToServer::login_mode,
            "username", &packet::game::OnSendToServer::username
        );

        lua.new_usertype<packet::game::OnNameChanged>("OnNameChangedPacket",
            sol::constructors<packet::game::OnNameChanged()>(),
            sol::base_classes, sol::bases<packet::IPacket>(),
            "name", &packet::game::OnNameChanged::name,
            "net_id", &packet::game::OnNameChanged::net_id
        );

        lua.new_usertype<packet::game::OnChangeSkin>("OnChangeSkinPacket",
            sol::constructors<packet::game::OnChangeSkin()>(),
            sol::base_classes, sol::bases<packet::IPacket>(),
            "skin", &packet::game::OnChangeSkin::skin,
            "net_id", &packet::game::OnChangeSkin::net_id
        );
    }

    void bind_send_functions(sol::state& lua)
    {
        auto send_table{ lua.create_table() };

        send_table.set_function("to_server", [this](packet::IPacket& pkt) {
            spdlog::debug("[Lua] Sending packet to server");
            return packet::PacketHelper::write(pkt, client_);
        });

        send_table.set_function("to_client", [this](packet::IPacket& pkt) {
            spdlog::debug("[Lua] Sending packet to client");
            return packet::PacketHelper::write(pkt, server_);
        });

        lua["send"] = send_table;

        auto packet_table{ lua.create_table() };

        packet_table.set_function("send_raw", [this](const sol::table& data_table, const bool to_server) {
            std::vector<std::byte> data;
            data.reserve(data_table.size());

            for (size_t i = 1; i <= data_table.size(); ++i) {
                sol::optional<int> byte_val = data_table[i];
                if (byte_val) {
                    data.push_back(static_cast<std::byte>(*byte_val));
                }
            }

            if (data.empty()) {
                spdlog::warn("[Lua] send_raw: empty data");
                return false;
            }

            return send_to_direction(data, to_server);
        });

        packet_table.set_function("send_text", [this](
            const std::string& text,
            const bool to_server,
            const sol::optional<int> msg_type_opt
        ) {
            return send_text_packet(text, to_server, msg_type_opt);
        });

        packet_table.set_function("send_text_parse", [this](
            const TextParse& text_parse,
            const bool to_server,
            const sol::optional<int> msg_type_opt
        ) {
            return send_text_packet(text_parse.get_raw(), to_server, msg_type_opt);
        });

        packet_table["NET_MESSAGE_GENERIC_TEXT"] = static_cast<int>(packet::NET_MESSAGE_GENERIC_TEXT);
        packet_table["NET_MESSAGE_GAME_MESSAGE"] = static_cast<int>(packet::NET_MESSAGE_GAME_MESSAGE);
        packet_table["NET_MESSAGE_GAME_PACKET"] = static_cast<int>(packet::NET_MESSAGE_GAME_PACKET);

        lua["packet"] = packet_table;
    }

    bool send_to_direction(const std::vector<std::byte>& data, const bool to_server)
    {
        if (to_server) {
            return client_.write(data);
        }

        return server_.write(data);
    }

    bool send_text_packet(
        const std::string& text,
        const bool to_server,
        const sol::optional<int> msg_type_opt
    ) {
        const packet::NetMessageType msg_type = msg_type_opt.has_value()
            ? static_cast<packet::NetMessageType>(*msg_type_opt)
            : packet::NetMessageType::NET_MESSAGE_GAME_MESSAGE;

        ByteStream stream;
        stream.write(magic_enum::enum_underlying(msg_type));
        stream.write(text, false);

        auto data = stream.get_data();
        data.push_back(static_cast<std::byte>(0x00));

        return send_to_direction(data, to_server);
    }

    network::Client& client_;
    network::Server& server_;
};
}
