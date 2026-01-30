#pragma once
#include "../packet_helper.hpp"

namespace packet::game {
struct OnNameChanged : VariantPacket<PacketId::OnNameChanged> {
    int32_t net_id;
    std::string name;

    OnNameChanged()
        : net_id{ - 1}
    {

    }

    bool read(const Payload& payload) override
    {
        const auto var{ get_payload_if<VariantPayload>(payload) };
        if (!var) {
            return false;
        }

        variant = var->variant;
        game_packet = var->game_packet;

        if (variant.size() < 2) {
            return false;
        }

        name = variant.get<std::string>(1);
        return true;
    }

    Payload write() override
    {
        game_packet.net_id = net_id;
        const PacketVariant variant{
            "OnNameChanged",
            name
        };
        return VariantPayload{ game_packet, variant };
    }
};

struct OnChangeSkin : VariantPacket<PacketId::OnChangeSkin> {
    int32_t net_id;
    uint32_t skin_code;

    OnChangeSkin()
        : net_id{ -1 }
        , skin_code{ 0 }
    {

    }

    bool read(const Payload& payload) override
    {
        const auto var{ get_payload_if<VariantPayload>(payload) };
        if (!var) {
            return false;
        }

        variant = var->variant;
        game_packet = var->game_packet;

        if (variant.size() < 2) {
            return false;
        }

        skin_code = variant.get<uint32_t>(1);
        return true;
    }

    Payload write() override
    {
        game_packet.net_id = net_id;
        const PacketVariant variant{
            "OnChangeSkin",
            skin_code
        };
        return VariantPayload{ game_packet, variant };
    }
};
}