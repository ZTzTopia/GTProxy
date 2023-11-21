#pragma once
#include <set>
#include <magic_enum.hpp>

#include "packet_types.hpp"
#include "../player/player.hpp"
#include "../utils/byte_stream.hpp"

namespace packet {
struct PacketHelper {
    template <class Packet>
    static bool send(Packet& packet, player::Player* player)
    {
        if constexpr (!is_net_message<Packet>::value && !is_net_packet<Packet>::value) {
            return false;
        }

        ByteStream byte_stream{};
        byte_stream.write(magic_enum::enum_underlying(Packet::MESSAGE_TYPE));

        if constexpr (is_net_message<Packet>::value) {
            packet.write(byte_stream);
        }
        else if constexpr (is_net_packet<Packet>::value) {
            GameUpdatePacket game_packet{};
            std::vector<std::byte> ext_data{};

            packet.write(game_packet, ext_data);

            game_packet.type = Packet::PACKET_TYPE;
            if (!ext_data.empty()) {
                game_packet.flags.extended = 1;
                game_packet.data_size = ext_data.size();
            }

            byte_stream.write(game_packet);
            byte_stream.write_data(ext_data.data(), ext_data.size());
        }

        return player->send_packet(byte_stream.get_data(), Packet::CHANNEL);
    }
    
    template <class Packet>
    static bool broadcastToSome(Packet& packet, const std::set<player::Player*>& players)
    {
        bool result{ true };
        for (const auto player : players) {
            result = send(packet, player);
        }

        return result;
    }

    template <class Packet>
    static bool broadcastToWorld(Packet &packet)
    {
        return false;
    }

    template <class Packet>
    static bool broadcast(Packet &packet)
    {
        return false;
    }
};
}
