#pragma once
#include "../packet_types.hpp"

namespace packet::game {
struct SendItemDatabase : NetPacket<PacketType::PACKET_SEND_ITEM_DATABASE_DATA> {
    std::vector<std::byte> data;
    std::uint32_t data_size;
    bool is_compressed;

    SendItemDatabase()
        : data_size{ 0 }
        , is_compressed{ false }
    {
    }

    void write(GameUpdatePacket& game_update_packet, std::vector<std::byte>& ext_data) override
    {
        game_update_packet.net_id = -1;
        if (is_compressed) {
            game_update_packet.decompressed_data_size = data_size;
        }

        ext_data = data;
    }
};
}
