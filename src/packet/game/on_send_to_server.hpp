#pragma once
#include <spdlog/spdlog.h>

#include "../packet_types.hpp"
#include "../packet_helper.hpp"
#include "../packet_variant.hpp"

namespace packet::game {
struct Disconnect : NetPacket<PacketType::PACKET_DISCONNECT> {
    void write(GameUpdatePacket& game_update_packet, std::vector<std::byte>& ext_data) override
    {
        game_update_packet.net_id = -1;
        ext_data.clear();
    }

    bool read(const PacketVariant& variant) override
    {
        return true;
    }
};

struct OnSendToServer : NetPacket<PacketType::PACKET_CALL_FUNCTION> {
    uint16_t port;
    int32_t token;
    int32_t user;
    std::string address;
    std::string door_id;
    std::string uuid_token;
    uint8_t login_mode;
    std::string username;

    void write(GameUpdatePacket& game_update_packet, std::vector<std::byte>& ext_data) override
    {
        game_update_packet.net_id = -1;

        TextParse text_parse{};
        text_parse.add(address, { door_id, uuid_token });

        const PacketVariant variant{
            "OnSendToServer",
            port,
            token,
            user,
            text_parse.get_raw(),
            login_mode,
            username
        };

        ext_data = variant.serialize();
    }

    bool read(const PacketVariant& variant) override
    {
        if (variant.size() < 5) {
            return false;
        }

        port = variant.get<uint32_t>(1);
        token = variant.get<int32_t>(2);
        user = variant.get<int32_t>(3);

        const std::string raw_text{ variant.get<std::string>(4) };
        const std::string key{ raw_text.substr(0, raw_text.find_first_of('|')) };

        TextParse text_parse{};
        text_parse.parse(raw_text);

        address = text_parse.get(key, 0);
        door_id = text_parse.get(key, 1);
        uuid_token = text_parse.get(key, 2);

        login_mode = variant.get<uint32_t>(5);
        username = variant.get<std::string>(6);
        return true;
    }
};
}
