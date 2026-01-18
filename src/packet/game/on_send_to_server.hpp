#pragma once
#include <spdlog/spdlog.h>

#include "../packet_helper.hpp"

namespace packet::game {
struct Disconnect : GamePacket<PacketId::Disconnect, PacketType::PACKET_DISCONNECT> {
    bool read(const Payload& payload) override
    {
        return is_payload<GamePayload>(payload);
    }

    Payload write() const override
    {
        GamePayload game_payload{};
        game_payload.packet.type = PACKET_TYPE;
        game_payload.packet.net_id = static_cast<uint32_t>(-1);
        return game_payload;
    }
};

struct OnSendToServer : VariantPacket<PacketId::OnSendToServer> {
    uint16_t port;
    int32_t token;
    int32_t user;
    std::string address;
    std::string door_id;
    std::string uuid_token;
    uint8_t login_mode;
    std::string username;

    bool read(const Payload& payload) override
    {
        const auto* var = get_payload_if<VariantPayload>(payload);
        if (!var) return false;
        
        const auto& variant = var->variant;
        if (variant.size() < 5) {
            return false;
        }

        port = variant.get<int32_t>(1);
        token = variant.get<int32_t>(2);
        user = variant.get<int32_t>(3);

        const std::string raw_text{ variant.get<std::string>(4) };
        const std::string key{ raw_text.substr(0, raw_text.find_first_of('|')) };

        TextParse text_parse{};
        text_parse.parse(raw_text);

        address = key;
        door_id = text_parse.get(key, 0);
        uuid_token = text_parse.get(key, 1);

        login_mode = variant.get<uint32_t>(5);
        username = variant.get<std::string>(6);
        return true;
    }

    Payload write() const override
    {
        TextParse text_parse{};
        text_parse.add(address, { door_id, uuid_token });

        PacketVariant variant{
            "OnSendToServer",
            static_cast<int32_t>(port),
            token,
            user,
            text_parse.get_raw(),
            static_cast<uint32_t>(login_mode),
            username
        };

        return VariantPayload{ std::move(variant) };
    }
};
}
