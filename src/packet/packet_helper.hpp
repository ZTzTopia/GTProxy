#pragma once
#include <set>
#include <magic_enum/magic_enum.hpp>

#include "packet_types.hpp"
#include "../player/player.hpp"
#include "../utils/byte_stream.hpp"
#include "../utils/text_parse.hpp"

namespace packet {
template <NetMessageType MsgType, int Channel = 0>
struct NetMessage {
    static constexpr NetMessageType MESSAGE_TYPE = MsgType;
    static constexpr int CHANNEL = Channel;

    [[nodiscard]] virtual bool read(TextParse&) const { return false; }
    virtual void write(ByteStream<>&) { }
};

std::false_type is_net_message_impl(...);
template <NetMessageType MsgType, int Channel>
std::true_type is_net_message_impl(NetMessage<MsgType, Channel> const volatile&);
// Get whether a type is a derived class of NetMessage
template <typename T>
using is_net_message = decltype(is_net_message_impl(std::declval<T const volatile&>()));

template <PacketType PktType, int Channel = 0>
struct NetPacket {
    static constexpr NetMessageType MESSAGE_TYPE = NET_MESSAGE_GAME_PACKET; // Always
    static constexpr PacketType PACKET_TYPE = PktType;
    static constexpr int CHANNEL = Channel;

    [[nodiscard]] virtual bool read(const GameUpdatePacket&) const { return false; }
    virtual void write(GameUpdatePacket&, std::vector<std::byte>&) { }
};

std::false_type is_net_packet_impl(...);
template <PacketType PktType, int Channel>
std::true_type is_net_packet_impl(NetPacket<PktType, Channel> const volatile&);
// Get whether a type is a derived class of NetPacket
template <typename T>
using is_net_packet = decltype(is_net_packet_impl(std::declval<T const volatile&>()));

struct PacketHelper {
    // Attempt to send a packet from derived class of NetMessage or NetPacket
    // to a player
    template <class Packet>
    static bool send(Packet& packet, const player::Player& player)
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

        return player.send_packet(byte_stream.get_data(), Packet::CHANNEL);
    }

    // Attempt to send a packet from derived class of NetMessage or NetPacket
    // to a set of players
    template <class Packet>
    static bool broadcastToSome(Packet& packet, const std::set<player::Player&>& players)
    {
        bool result{ true };
        for (const auto player : players) {
            if ((result = send(packet, player)) == false) {
                break;
            }
        }

        return result;
    }

    // Attempt to send a packet from derived class of NetMessage or NetPacket
    // to all players in a world
    template <class Packet>
    static bool broadcastToWorld(Packet &packet)
    {
        return false;
    }

    // Attempt to send a packet from derived class of NetMessage or NetPacket
    // to all players
    template <class Packet>
    static bool broadcast(Packet &packet)
    {
        return false;
    }
};
}
