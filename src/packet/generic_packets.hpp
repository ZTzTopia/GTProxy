#pragma once
#include "packet_helper.hpp"
#include "packet_id.hpp"
#include "payload.hpp"

namespace packet {
struct GenericTextPacket : TextPacket<PacketId::Unknown> {
    NetMessageType message_type{ NET_MESSAGE_GAME_MESSAGE };

    [[nodiscard]] bool read(const Payload& payload) override
    {
        const auto* text = get_payload_if<TextPayload>(payload);
        if (!text) {
            return false;
        }

        text_parse = text->data;
        message_type = text->message_type;
        raw_data = text->raw_data;
        return true;
    }

    [[nodiscard]] Payload write() override
    {
        return TextPayload{ message_type, text_parse };
    }
};

struct GenericVariantPacket : VariantPacket<PacketId::Unknown> {
    [[nodiscard]] bool read(const Payload& payload) override
    {
        const auto* var = get_payload_if<VariantPayload>(payload);
        if (!var) {
            return false;
        }

        variant = var->variant;
        game_packet = var->game_packet;
        raw_data = var->raw_data;
        return true;
    }

    [[nodiscard]] Payload write() override
    {
        return VariantPayload{ game_packet, variant };
    }
};

struct GenericGamePacket : GamePacket<PacketId::Unknown, PACKET_STATE> {
    [[nodiscard]] bool read(const Payload& payload) override
    {
        const auto* game{ get_payload_if<GamePayload>(payload) };
        if (!game) {
            return false;
        }

        game_packet = game->packet;
        extra = game->extra;
        raw_data = game->raw_data;
        return true;
    }

    [[nodiscard]] Payload write() override
    {
        return GamePayload{ game_packet, extra };
    }
};
}
