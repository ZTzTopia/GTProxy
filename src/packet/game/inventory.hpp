#pragma once
#include "../packet_helper.hpp"
#include "../packet_id.hpp"
#include "../packet_types.hpp"
#include "../payload.hpp"

namespace packet::game {
struct SendInventoryState : GamePacket<PacketId::SendInventoryState, PACKET_SEND_INVENTORY_STATE> {
    std::vector<std::byte> extra;

    [[nodiscard]] bool read(const Payload& payload) override
    {
        const auto* game_payload = std::get_if<GamePayload>(&payload);
        if (!game_payload) {
            return false;
        }

        extra = game_payload->extra;
        return true;
    }

    [[nodiscard]] Payload write() override
    {
        if (extra.empty()) {
            return Payload{};
        }

        GamePayload game_payload{};
        game_payload.packet.type = PACKET_TYPE;
        game_payload.packet.net_id = -1;
        game_payload.extra = extra;

        return game_payload;
    }
};

struct ModifyItemInventory : GamePacket<PacketId::ModifyItemInventory, PACKET_MODIFY_ITEM_INVENTORY> {
    int32_t item_id{ 0 };
    int32_t amount{ 0 };
    int32_t net_id{ -1 };

    [[nodiscard]] bool read(const Payload& payload) override
    {
        const auto* game_payload = std::get_if<GamePayload>(&payload);
        if (!game_payload) {
            return false;
        }

        game_packet = game_payload->packet;
        item_id = game_payload->packet.item_id;
        amount = static_cast<int32_t>(game_payload->packet.int_data);
        net_id = game_payload->packet.net_id;
        return true;
    }

    [[nodiscard]] Payload write() override
    {
        if (item_id == 0) {
            return Payload{};
        }

        GamePayload game_payload{};
        game_payload.packet.type = PACKET_TYPE;
        game_payload.packet.item_id = item_id;
        game_payload.packet.int_data = amount;
        game_payload.packet.net_id = net_id;
        return game_payload;
    }
};
}
