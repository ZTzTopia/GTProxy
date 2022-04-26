#pragma once
#include <string>
#include <enet/enet.h>
#include <util/Variant.h>

#include "packet.h"
#include "NetAvatar.h"

namespace server {
    class Server;
}

namespace player {
    class Player {
    public:
        explicit Player(ENetPeer *peer);
        ~Player() = default;

        int send_packet(eNetMessageType type, const std::string &data);
        int send_packet_packet(ENetPacket *packet);
        int send_raw_packet(eNetMessageType type, GameUpdatePacket *game_update_packet, size_t length = sizeof(GameUpdatePacket) - 4, uint8_t *extended_data = nullptr, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
        int send_variant(VariantList &&variant_list, uint32_t net_id = -1, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
        int send_log(const std::string &log, bool on_console_message = false);

        [[nodiscard]] ENetPeer *get_peer() const { return m_peer; }
        NetAvatar *get_avatar() { return m_avatar; }
    private:
        ENetPeer *m_peer;
        NetAvatar *m_avatar;
    };
}
