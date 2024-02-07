#include "player.hpp"

namespace player {
bool Player::send_packet(const std::vector<std::byte>& data, const int channel) const
{
    if (data.size() < 4 || data.size() > 786432 /* 768kb should be enough */) {
        return false;
    }

    ENetPacket* packet{ enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE) };
    if (const int ret{ enet_peer_send(peer_, channel, packet) }; ret != 0) {
        enet_packet_destroy(packet);
        return false;
    }

    return true;
}
}
