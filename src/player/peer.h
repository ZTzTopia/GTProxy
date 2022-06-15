#pragma once
#include <string>
#include <enet/enet.h>
#include <util/Variant.h>

#include "packet.h"

namespace player {
    class Peer {
    public:
        explicit Peer(ENetPeer* peer);
        ~Peer() = default;

        [[nodiscard]] bool is_connected() const { return m_peer->state == ENET_PEER_STATE_CONNECTED; }
        void disconnect() const { enet_peer_disconnect(m_peer, 0); }
        void disconnect_now() const { enet_peer_disconnect_now(m_peer, 0); }

        int send_packet(eNetMessageType type, const std::string& data);
        int send_packet_packet(ENetPacket* packet);
        int send_raw_packet(eNetMessageType type, GameUpdatePacket* game_update_packet, std::size_t length = sizeof(GameUpdatePacket), uint8_t* extended_data = nullptr, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
        int send_variant(VariantList&& variant_list, uint32_t net_id = -1, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);

    private:
        ENetPeer* m_peer;
    };
}
