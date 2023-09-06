#include "player.hpp"
#include "../packet/packet_helper.hpp"

namespace player {
Player::Player(ENetPeer* peer)
    : peer_{ peer }
{

}

bool Player::send_packet(const std::vector<std::byte>& data, int channel)
{
    if (data.size() < 4 || data.size() > 786432 /* 768kb should be enough */) {
        return false;
    }

    ENetPacket* packet{ enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE) };

    int ret{ enet_peer_send(peer_, channel, packet) };
    if (ret != 0) {
        enet_packet_destroy(packet);
        return false;
    }

    return true;
}
}
