#pragma once
#include "../packet_types.hpp"
#include "../packet_variant.hpp"

namespace packet::game {
struct OnSendToServer : NetPacket<PacketType::PACKET_CALL_FUNCTION> {
    uint16_t port;
    int32_t token;
    int32_t user;
    std::string address;
    std::string door_id;
    std::string uuid_token;
    uint8_t login_mode;

    void write(GameUpdatePacket& game_update_packet, std::vector<std::byte>& ext_data)
    {
        game_update_packet.net_id = -1;

        TextParse text_parse{};
        text_parse.add(address, { door_id, uuid_token });

        const Variant variant{
            "OnSendToServer",
            port,
            token,
            user,
            text_parse.get_raw(),
            login_mode
        };

        ext_data = variant.serialize();
    }
};
}
