#include <memory>

#include "peer.h"
#include "../utils/random.h"

namespace player {
Peer::Peer(ENetPeer* peer)
    : m_peer{ peer }
{
    static randutils::pcg_rng rng{ utils::random::get_generator_local() };
    std::string uid{ utils::random::generate_unicode(rng, 32) };

    m_peer->data = new std::uint8_t[32];
    std::memcpy(m_peer->data, &uid[0], uid.length());
}

Peer::~Peer()
{
    delete reinterpret_cast<uint8_t*>(m_peer->data);
}

int Peer::send_packet(eNetMessageType type, const std::string& data)
{
    if (!m_peer) {
        return -1;
    }

    std::vector<std::byte> packet_data(sizeof(type) + data.length());
    std::memcpy(packet_data.data(), &type, sizeof(type));
    std::memcpy(packet_data.data() + sizeof(type), data.c_str(), data.length());

    ENetPacket* packet{ enet_packet_create(packet_data.data(), packet_data.size(), ENET_PACKET_FLAG_RELIABLE) };
    

    return send_packet_packet(packet);
}

int Peer::send_packet_packet(ENetPacket* packet)
{
    if (!m_peer) {
        return -1;
    }
    send_lock.lock();
    int ret = enet_peer_send(m_peer, 0, packet);
    send_lock.unlock();
    if (ret != 0) {
        enet_packet_destroy(packet);
    }

    return ret;
}

int Peer::send_raw_packet(
    eNetMessageType type,
    GameUpdatePacket* game_update_packet,
    size_t length,
    uint8_t* extended_data,
    enet_uint32 flags
) {
    if (!m_peer) {
        return -1;
    }

    if (length > 1000000) {
        return -1;
    }

    ENetPacket* packet;
    if (type == NET_MESSAGE_GAME_PACKET && game_update_packet->flags.bExtended) {
        packet = enet_packet_create(nullptr, length + game_update_packet->data_size + 5, flags);
        std::memcpy(packet->data, &type, sizeof(eNetMessageType));
        std::memcpy(packet->data + sizeof(eNetMessageType), game_update_packet, length);
        std::memcpy(packet->data + length + sizeof(eNetMessageType), extended_data, game_update_packet->data_size);
    }
    else {
        packet = enet_packet_create(nullptr, length + 5, flags);
        std::memcpy(packet->data, &type, sizeof(eNetMessageType));
        std::memcpy(packet->data + sizeof(eNetMessageType), game_update_packet, length);
    }

    
    return send_packet_packet(packet);
}

int Peer::send_variant(VariantList&& variant_list, std::uint32_t net_id, enet_uint32 flags)
{
    if (variant_list.Get(0).GetType() == eVariantType::TYPE_UNUSED) {
        return -1;
    }

    GameUpdatePacket game_update_packet{};
    game_update_packet.type = ePacketType::PACKET_CALL_FUNCTION;
    game_update_packet.net_id = net_id;
    game_update_packet.flags.bExtended = true;
    game_update_packet.data_size = 0;

    std::uint8_t* data{ variant_list.SerializeToMem(&game_update_packet.data_size, nullptr) };
    variant_list.Reset();

    int ret{ send_raw_packet(eNetMessageType::NET_MESSAGE_GAME_PACKET, &game_update_packet, sizeof(GameUpdatePacket), data, flags) };
    delete data;
    return ret;
}
}