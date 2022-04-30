#include <memory>

#include "player.h"

namespace player {
    Player::Player(ENetPeer *peer)
        : m_peer(peer), m_fast_drop(false)
    {
        peer->data = reinterpret_cast<void *>(this);
        this->m_avatar = new NetAvatar();
        m_player_items = new PlayerItems{};
        m_world = new World{};
    }

    int Player::send_packet(eNetMessageType type, const std::string &data) {
        if (!m_peer)
            return -1;
        ENetPacket *packet = enet_packet_create(nullptr, data.length() + 5, ENET_PACKET_FLAG_RELIABLE);
        *(eNetMessageType *)packet->data = type;
        std::memcpy(packet->data + sizeof(eNetMessageType), data.c_str(), data.length());

        int ret = enet_peer_send(m_peer, 0, packet) != 0;
        if (ret) enet_packet_destroy(packet);
        return ret;
    }
    int Player::send_packet_packet(ENetPacket *packet) {
        if (!m_peer)
            return -1;
        ENetPacket *packet_ = enet_packet_create(nullptr, packet->dataLength, packet->flags);
        std::memcpy(packet_->data, packet->data, packet->dataLength);

        int ret = enet_peer_send(m_peer, 0, packet_) != 0;
        if (ret) enet_packet_destroy(packet);
        return ret;
    }
    int Player::send_raw_packet(eNetMessageType type, GameUpdatePacket *game_update_packet, size_t length, uint8_t *extended_data, enet_uint32 flags) {
        if (!m_peer)
            return -1;
        if (length > 0xF4240)
            return -1;
        int ret = -1;
        if (type == NET_MESSAGE_GAME_PACKET && (game_update_packet->flags & player::PACKET_FLAG_EXTENDED)) {
            ENetPacket *packet = enet_packet_create(nullptr, length + game_update_packet->data_size + 5, flags);
            *(eNetMessageType *)packet->data = type;
            std::memcpy(packet->data + sizeof(eNetMessageType), game_update_packet, length);
            std::memcpy(packet->data + length + sizeof(eNetMessageType), extended_data, game_update_packet->data_size);

            ret = enet_peer_send(m_peer, 0, packet) != 0;
            if (ret)
                enet_packet_destroy(packet);
        }
        else {
            ENetPacket *packet = enet_packet_create(nullptr, length + 5, flags);
            *(eNetMessageType *) packet->data = type;
            std::memcpy(packet->data + sizeof(eNetMessageType), game_update_packet, length);

            ret = enet_peer_send(m_peer, 0, packet) != 0;
            if (ret)
                enet_packet_destroy(packet);
        }
        return ret;
    }
    int Player::send_variant(VariantList &&variant_list, uint32_t net_id, enet_uint32 flags) {
        if (variant_list.Get(0).GetType() == eVariantType::TYPE_UNUSED)
            return -1;

        uint32_t data_size;
        uint8_t *data = variant_list.SerializeToMem(&data_size, nullptr);
        GameUpdatePacket game_update_packet{};
        game_update_packet.type = PACKET_CALL_FUNCTION;
        game_update_packet.net_id = net_id;
        game_update_packet.flags |= player::PACKET_FLAG_EXTENDED;
        game_update_packet.data_size = data_size;

        int ret{ send_raw_packet(NET_MESSAGE_GAME_PACKET, &game_update_packet, sizeof(GameUpdatePacket), data, flags) };
        delete data;
        return ret;
    }
    int Player::send_log(const std::string &log, bool on_console_message) {
        if (!on_console_message)
            return send_packet(NET_MESSAGE_GAME_MESSAGE, fmt::format("action|log\nmsg|{}", log));
        return send_variant({ "OnConsoleMessage", log });
    }
}