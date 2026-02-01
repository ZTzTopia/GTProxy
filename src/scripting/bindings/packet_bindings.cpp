#include "packet_bindings.hpp"
#include "../../packet/generic_packets.hpp"
#include "../../packet/packet_variant.hpp"

namespace scripting::bindings {
void PacketBindings::bind(sol::state& lua)
{
    bind_enums(lua);
    bind_text_parse(lua);
    bind_base_packet(lua);
    bind_message_packets(lua);
    bind_game_packets(lua);
    bind_generic_packets(lua);
    bind_game_update_packet(lua);
    bind_packet_variant(lua);
    bind_send_functions(lua);
}

void PacketBindings::bind_enums(sol::state& lua)
{
    auto packet_table = lua["packet"].get_or(lua.create_table());

    auto type_table = lua.create_table();
    constexpr auto type_values = magic_enum::enum_values<packet::PacketType>();
    for (const auto v : type_values) {
        type_table[magic_enum::enum_name(v)] = static_cast<uint8_t>(v);
    }
    packet_table["PacketType"] = type_table;

    auto flag_table = lua.create_table();
    flag_table["PACKET_FLAG_NONE"] = static_cast<uint32_t>(packet::PACKET_FLAG_NONE);
    flag_table["PACKET_FLAG_UNK"] = static_cast<uint32_t>(packet::PACKET_FLAG_UNK);
    flag_table["PACKET_FLAG_RESET_VISUAL_STATE"] = static_cast<uint32_t>(packet::PACKET_FLAG_RESET_VISUAL_STATE);
    flag_table["PACKET_FLAG_EXTENDED"] = static_cast<uint32_t>(packet::PACKET_FLAG_EXTENDED);
    flag_table["PACKET_FLAG_ROTATE_LEFT"] = static_cast<uint32_t>(packet::PACKET_FLAG_ROTATE_LEFT);
    flag_table["PACKET_FLAG_ON_SOLID"] = static_cast<uint32_t>(packet::PACKET_FLAG_ON_SOLID);
    flag_table["PACKET_FLAG_ON_FIRE_DAMAGE"] = static_cast<uint32_t>(packet::PACKET_FLAG_ON_FIRE_DAMAGE);
    flag_table["PACKET_FLAG_ON_JUMP"] = static_cast<uint32_t>(packet::PACKET_FLAG_ON_JUMP);
    flag_table["PACKET_FLAG_ON_KILLED"] = static_cast<uint32_t>(packet::PACKET_FLAG_ON_KILLED);
    flag_table["PACKET_FLAG_ON_PUNCHED"] = static_cast<uint32_t>(packet::PACKET_FLAG_ON_PUNCHED);
    flag_table["PACKET_FLAG_ON_PLACED"] = static_cast<uint32_t>(packet::PACKET_FLAG_ON_PLACED);
    flag_table["PACKET_FLAG_ON_TILE_ACTION"] = static_cast<uint32_t>(packet::PACKET_FLAG_ON_TILE_ACTION);
    flag_table["PACKET_FLAG_ON_GOT_PUNCHED"] = static_cast<uint32_t>(packet::PACKET_FLAG_ON_GOT_PUNCHED);
    flag_table["PACKET_FLAG_ON_RESPAWNED"] = static_cast<uint32_t>(packet::PACKET_FLAG_ON_RESPAWNED);
    flag_table["PACKET_FLAG_ON_COLLECT_OBJECT"] = static_cast<uint32_t>(packet::PACKET_FLAG_ON_COLLECT_OBJECT);
    flag_table["PACKET_FLAG_ON_TRAMPOLINE"] = static_cast<uint32_t>(packet::PACKET_FLAG_ON_TRAMPOLINE);
    flag_table["PACKET_FLAG_ON_DAMAGE"] = static_cast<uint32_t>(packet::PACKET_FLAG_ON_DAMAGE);
    flag_table["PACKET_FLAG_ON_SLIDE"] = static_cast<uint32_t>(packet::PACKET_FLAG_ON_SLIDE);
    flag_table["PACKET_FLAG_ON_WALL_HANG"] = static_cast<uint32_t>(packet::PACKET_FLAG_ON_WALL_HANG);
    flag_table["PACKET_FLAG_ON_ACID_DAMAGE"] = static_cast<uint32_t>(packet::PACKET_FLAG_ON_ACID_DAMAGE);
    packet_table["PacketFlag"] = flag_table;

    lua["packet"] = packet_table;
}

void PacketBindings::bind_text_parse(sol::state& lua)
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

void PacketBindings::bind_base_packet(sol::state& lua)
{
    lua.new_usertype<packet::IPacket>("IPacket",
        sol::no_constructor,
        "id", &packet::IPacket::id,
        "channel", &packet::IPacket::channel,
        "has_raw_data", &packet::IPacket::has_raw_data,
        "raw", sol::property([](const packet::IPacket& p, sol::this_state s) {
            sol::state_view lua{ s };
            sol::table t = lua.create_table();
            for (std::size_t i = 0; i < p.raw_data.size(); ++i) {
                t[i + 1] = static_cast<uint8_t>(p.raw_data[i]);
            }
            return t;
        })
    );
}

void PacketBindings::bind_message_packets(sol::state& lua)
{
    lua.new_usertype<packet::message::ServerHello>("ServerHelloPacket",
        sol::constructors<packet::message::ServerHello()>(),
        sol::base_classes, sol::bases<packet::IPacket>(),
        "text_parse", &packet::message::ServerHello::text_parse
    );

    lua.new_usertype<packet::message::Log>("LogPacket",
        sol::constructors<packet::message::Log()>(),
        sol::base_classes, sol::bases<packet::IPacket>(),
        "msg", &packet::message::Log::msg,
        "text_parse", &packet::message::Log::text_parse
    );

    lua.new_usertype<packet::message::Quit>("QuitPacket",
        sol::constructors<packet::message::Quit()>(),
        sol::base_classes, sol::bases<packet::IPacket>(),
        "text_parse", &packet::message::Quit::text_parse
    );

    lua.new_usertype<packet::message::QuitToExit>("QuitToExitPacket",
        sol::constructors<packet::message::QuitToExit()>(),
        sol::base_classes, sol::bases<packet::IPacket>(),
        "text_parse", &packet::message::QuitToExit::text_parse
    );

    lua.new_usertype<packet::message::JoinRequest>("JoinRequestPacket",
        sol::constructors<packet::message::JoinRequest()>(),
        sol::base_classes, sol::bases<packet::IPacket>(),
        "world_name", &packet::message::JoinRequest::world_name,
        "invited_world", &packet::message::JoinRequest::invited_world,
        "text_parse", &packet::message::JoinRequest::text_parse
    );

    lua.new_usertype<packet::message::ValidateWorld>("ValidateWorldPacket",
        sol::constructors<packet::message::ValidateWorld()>(),
        sol::base_classes, sol::bases<packet::IPacket>(),
        "world_name", &packet::message::ValidateWorld::world_name,
        "text_parse", &packet::message::ValidateWorld::text_parse
    );

    lua.new_usertype<packet::message::Input>("InputPacket",
        sol::constructors<packet::message::Input()>(),
        sol::base_classes, sol::bases<packet::IPacket>(),
        "text", &packet::message::Input::text,
        "text_parse", &packet::message::Input::text_parse
    );
}

void PacketBindings::bind_game_packets(sol::state& lua)
{
    lua.new_usertype<packet::game::Disconnect>("DisconnectPacket",
        sol::constructors<packet::game::Disconnect()>(),
        sol::base_classes, sol::bases<packet::IPacket>(),
        "game_packet", &packet::game::Disconnect::game_packet,
        "extra", sol::property([](const packet::game::Disconnect& p, sol::this_state s) {
            sol::state_view lua{ s };
            sol::table t = lua.create_table();
            for (std::size_t i = 0; i < p.extra.size(); ++i) {
                t[i + 1] = static_cast<uint8_t>(p.extra[i]);
            }
            return t;
        })
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
        "username", &packet::game::OnSendToServer::username,
        "variant", &packet::game::OnSendToServer::variant,
        "game_packet", &packet::game::OnSendToServer::game_packet
    );

    lua.new_usertype<packet::game::OnNameChanged>("OnNameChangedPacket",
        sol::constructors<packet::game::OnNameChanged()>(),
        sol::base_classes, sol::bases<packet::IPacket>(),
        "name", &packet::game::OnNameChanged::name,
        "net_id", &packet::game::OnNameChanged::net_id,
        "variant", &packet::game::OnNameChanged::variant,
        "game_packet", &packet::game::OnNameChanged::game_packet
    );

    lua.new_usertype<packet::game::OnChangeSkin>("OnChangeSkinPacket",
        sol::constructors<packet::game::OnChangeSkin()>(),
        sol::base_classes, sol::bases<packet::IPacket>(),
        "skin", &packet::game::OnChangeSkin::skin_code,
        "net_id", &packet::game::OnChangeSkin::net_id,
        "variant", &packet::game::OnChangeSkin::variant,
        "game_packet", &packet::game::OnChangeSkin::game_packet
    );

    lua.new_usertype<packet::game::OnSpawn>("OnSpawnPacket",
        sol::constructors<packet::game::OnSpawn()>(),
        sol::base_classes, sol::bases<packet::IPacket>(),
        "spawn", &packet::game::OnSpawn::spawn,
        "net_id", &packet::game::OnSpawn::net_id,
        "user_id", &packet::game::OnSpawn::user_id,
        "country_code", &packet::game::OnSpawn::country_code,
        "name", &packet::game::OnSpawn::name,
        "position", sol::property([](const packet::game::OnSpawn& p, sol::this_state s) {
            sol::state_view lua{ s };
            sol::table t = lua.create_table();
            t["x"] = p.position.x;
            t["y"] = p.position.y;
            return t;
        }),
        "invisible", &packet::game::OnSpawn::invisible,
        "mod_state", &packet::game::OnSpawn::mod_state,
        "supermod_state", &packet::game::OnSpawn::supermod_state,
        "online_id", &packet::game::OnSpawn::online_id,
        "type", &packet::game::OnSpawn::type,
        "title_icon", &packet::game::OnSpawn::title_icon,
        "variant", &packet::game::OnSpawn::variant,
        "game_packet", &packet::game::OnSpawn::game_packet
    );

    lua.new_usertype<packet::game::OnRemove>("OnRemovePacket",
        sol::constructors<packet::game::OnRemove()>(),
        sol::base_classes, sol::bases<packet::IPacket>(),
        "net_id", &packet::game::OnRemove::net_id,
        "player_id", &packet::game::OnRemove::player_id,
        "variant", &packet::game::OnRemove::variant,
        "game_packet", &packet::game::OnRemove::game_packet
    );
}

void PacketBindings::bind_generic_packets(sol::state& lua)
{
    lua.new_usertype<packet::GenericTextPacket>("GenericTextPacket",
        sol::constructors<packet::GenericTextPacket()>(),
        sol::base_classes, sol::bases<packet::IPacket>(),
        "text_parse", &packet::GenericTextPacket::text_parse,
        "message_type", &packet::GenericTextPacket::message_type
    );

    lua.new_usertype<packet::GenericVariantPacket>("GenericVariantPacket",
        sol::constructors<packet::GenericVariantPacket()>(),
        sol::base_classes, sol::bases<packet::IPacket>(),
        "variant", &packet::GenericVariantPacket::variant,
        "game_packet", &packet::GenericVariantPacket::game_packet
    );

    lua.new_usertype<packet::GenericGamePacket>("GenericGamePacket",
        sol::constructors<packet::GenericGamePacket()>(),
        sol::base_classes, sol::bases<packet::IPacket>(),
        "game_packet", &packet::GenericGamePacket::game_packet,
        "extra", sol::property([](const packet::GenericGamePacket& p, sol::this_state s) {
            sol::state_view lua{ s };
            sol::table t = lua.create_table();
            for (std::size_t i = 0; i < p.extra.size(); ++i) {
                t[i + 1] = static_cast<uint8_t>(p.extra[i]);
            }
            return t;
        })
    );
}

void PacketBindings::bind_game_update_packet(sol::state& lua)
{
    lua.new_usertype<packet::GameUpdatePacket>("GameUpdatePacket",
        sol::constructors<packet::GameUpdatePacket()>(),
        "type", &packet::GameUpdatePacket::type,
        "net_id", &packet::GameUpdatePacket::net_id,
        "flags", sol::property([](const packet::GameUpdatePacket& p) { return p.flags.value; }),
        "data_size", &packet::GameUpdatePacket::data_size,
        "pos_x", sol::property(
            [](const packet::GameUpdatePacket& p) { return *reinterpret_cast<const float*>(&p.pad_4[0]); },
            [](packet::GameUpdatePacket& p, float v) { *reinterpret_cast<float*>(&p.pad_4[0]) = v; }
        ),
        "pos_y", sol::property(
            [](const packet::GameUpdatePacket& p) { return *reinterpret_cast<const float*>(&p.pad_4[4]); },
            [](packet::GameUpdatePacket& p, float v) { *reinterpret_cast<float*>(&p.pad_4[4]) = v; }
        ),
        "vec_x", sol::property(
            [](const packet::GameUpdatePacket& p) { return *reinterpret_cast<const float*>(&p.pad_4[8]); },
            [](packet::GameUpdatePacket& p, float v) { *reinterpret_cast<float*>(&p.pad_4[8]) = v; }
        ),
        "vec_y", sol::property(
            [](const packet::GameUpdatePacket& p) { return *reinterpret_cast<const float*>(&p.pad_4[12]); },
            [](packet::GameUpdatePacket& p, float v) { *reinterpret_cast<float*>(&p.pad_4[12]) = v; }
        ),
        "modifier", sol::property(
            [](const packet::GameUpdatePacket& p) { return *reinterpret_cast<const float*>(&p.pad_4[16]); },
            [](packet::GameUpdatePacket& p, float v) { *reinterpret_cast<float*>(&p.pad_4[16]) = v; }
        ),
        "int_data", sol::property(
            [](const packet::GameUpdatePacket& p) { return *reinterpret_cast<const int32_t*>(&p.pad_4[24]); },
            [](packet::GameUpdatePacket& p, int32_t v) { *reinterpret_cast<int32_t*>(&p.pad_4[24]) = v; }
        )
    );
}

void PacketBindings::bind_packet_variant(sol::state& lua)
{
    auto packet_table = lua["packet"].get_or(lua.create_table());

    auto variant_type_table = lua.create_table();
    constexpr auto variant_type_values = magic_enum::enum_values<packet::VariantType>();
    for (const auto v : variant_type_values) {
        variant_type_table[magic_enum::enum_name(v)] = static_cast<uint8_t>(v);
    }
    packet_table["VariantType"] = variant_type_table;
    lua["packet"] = packet_table;

    lua.new_usertype<packet::PacketVariant>("PacketVariant",
        sol::constructors<packet::PacketVariant()>(),

        "get", [](const packet::PacketVariant& var, std::size_t index, sol::this_state s) -> sol::object {
            if (index >= var.size()) {
                return sol::make_object(s, sol::lua_nil);
            }

            const auto& variants = var.get_variants();
            const auto& v = variants[index];
            const auto type = packet::PacketVariant::get_type(v);

            switch (type) {
            case packet::VariantType::FLOAT:
                return sol::make_object(s, std::get<float>(v));
            case packet::VariantType::STRING:
                return sol::make_object(s, std::get<std::string>(v));
            case packet::VariantType::VEC2: {
                const auto& vec = std::get<glm::vec2>(v);
                sol::state_view lua{ s };
                sol::table t = lua.create_table();
                t["x"] = vec.x;
                t["y"] = vec.y;
                return t;
            }
            case packet::VariantType::VEC3: {
                const auto& vec = std::get<glm::vec3>(v);
                sol::state_view lua{ s };
                sol::table t = lua.create_table();
                t["x"] = vec.x;
                t["y"] = vec.y;
                t["z"] = vec.z;
                return t;
            }
            case packet::VariantType::UNSIGNED:
                return sol::make_object(s, std::get<uint32_t>(v));
            case packet::VariantType::SIGNED:
                return sol::make_object(s, std::get<int32_t>(v));
            default:
                return sol::make_object(s, sol::lua_nil);
            }
        },

        "get_string", [](const packet::PacketVariant& var, std::size_t index) {
            return var.get<std::string>(index);
        },
        "get_int", [](const packet::PacketVariant& var, std::size_t index) {
            return var.get<int32_t>(index);
        },
        "get_uint", [](const packet::PacketVariant& var, std::size_t index) {
            return var.get<uint32_t>(index);
        },
        "get_float", [](const packet::PacketVariant& var, std::size_t index) {
            return var.get<float>(index);
        },
        "get_vec2", [](const packet::PacketVariant& var, std::size_t index, sol::this_state s) {
            sol::state_view lua{ s };
            const auto vec = var.get<glm::vec2>(index);
            sol::table t = lua.create_table();
            t["x"] = vec.x;
            t["y"] = vec.y;
            return t;
        },
        "get_vec3", [](const packet::PacketVariant& var, std::size_t index, sol::this_state s) {
            sol::state_view lua{ s };
            const auto vec = var.get<glm::vec3>(index);
            sol::table t = lua.create_table();
            t["x"] = vec.x;
            t["y"] = vec.y;
            t["z"] = vec.z;
            return t;
        },

        "size", &packet::PacketVariant::size,
        "type_at", [](const packet::PacketVariant& var, std::size_t index) -> std::string {
            if (index >= var.size()) {
                return "nil";
            }
            const auto& variants = var.get_variants();
            const auto type = packet::PacketVariant::get_type(variants[index]);
            return std::string(magic_enum::enum_name(type));
        }
    );
}

void PacketBindings::bind_send_functions(sol::state& lua)
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

    auto packet_table{ lua["packet"].get_or(lua.create_table()) };

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

bool PacketBindings::send_to_direction(const std::vector<std::byte>& data, const bool to_server)
{
    if (to_server) {
        return client_.write(data);
    }

    return server_.write(data);
}

bool PacketBindings::send_text_packet(
    const std::string& text,
    const bool to_server,
    const sol::optional<int> msg_type_opt
)
{
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
}
