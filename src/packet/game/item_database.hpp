#pragma once
#include <spdlog/spdlog.h>

#include "../packet_helper.hpp"
#include "../packet_id.hpp"
#include "../packet_types.hpp"
#include "../payload.hpp"
#include "../../item/item_database.hpp"
#include "../../utils/zlib.hpp"

namespace packet::game {
struct SendItemDatabaseData : GamePacket<PacketId::SendItemDatabaseData, PACKET_SEND_ITEM_DATABASE_DATA> {
    std::vector<std::byte> items_dat;

    [[nodiscard]] bool read(const Payload& payload) override
    {
        const auto game_payload{ std::get_if<GamePayload>(&payload) };
        if (!game_payload) {
            return false;
        }

        if (game_payload->extra.empty()) {
            return false;
        }

        const std::uint32_t decompressed_size{ game_payload->packet.decompressed_data_size };
        if (decompressed_size == 0) {
            return false;
        }

        if (!utils::decompress_zlib(game_payload->extra, items_dat, decompressed_size)) {
            return false;
        }

        return true;
    }

    [[nodiscard]] Payload write() override
    {
        if (items_dat.empty()) {
            return Payload{};
        }

        GamePayload game_payload{};
        game_payload.packet.type = PACKET_TYPE;
        game_payload.packet.net_id = -1;

        std::vector<std::byte> compressed_data{};
        if (!utils::compress_zlib(items_dat, compressed_data)) {
            spdlog::warn("Failed to compress items.dat data");
            return Payload{};
        }

        game_payload.packet.decompressed_data_size = static_cast<std::uint32_t>(items_dat.size());
        game_payload.extra = std::move(compressed_data);

        return game_payload;
    }
};
}
