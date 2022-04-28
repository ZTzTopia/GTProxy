#pragma once
#include <string>
#include <map>
#include <unordered_map>
#include <enet/enet.h>
#include <util/Variant.h>

#include "packet.h"
#include "NetAvatar.h"

namespace server {
    class Server;
}

namespace player {
    struct Inventory {
        uint32_t size;
        std::map<uint16_t, uint8_t> items;
    };

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
        std::unordered_map<int32_t, NetAvatar*> &get_avatar_map() { return m_avatar_map; }
        Inventory* get_inventory() { return m_inventory; }
    private:
        ENetPeer *m_peer;
        NetAvatar *m_avatar;
        std::unordered_map<int32_t, NetAvatar*> m_avatar_map;
        Inventory *m_inventory;
    };
}
