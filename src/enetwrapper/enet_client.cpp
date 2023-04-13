#include <string>

#include "enet_client.h"

namespace enet_wrapper {
ENetClient::ENetClient()
    : m_host(nullptr)
    , m_peer(nullptr)
{
    m_running.store(false);
}

ENetClient::~ENetClient()
{
    destroy_host();
}

bool ENetClient::create_host(std::size_t peer_count, enet_uint8 using_new_packet)
{
    if (m_host) {
        destroy_host();
    }

    m_host = enet_host_create(nullptr, peer_count, 2, 0, 0);
    if (!m_host) {
        return false;
    }

    if (enet_host_compress_with_range_coder(m_host) != 0) {
        return false;
    }

    m_host->checksum = enet_crc32;
    m_host->usingNewPacket = using_new_packet;
    return true;
}

void ENetClient::destroy_host()
{
    if (m_running.load()) {
        m_running.store(false);
        m_service_thread.join();
    }

    if (m_host) {
        enet_host_destroy(m_host);
        m_host = nullptr;
    }
}

bool ENetClient::connect(const std::string& host, enet_uint16 port)
{
    if (!m_host) {
        return false;
    }

    ENetAddress address{};
    enet_address_set_host(&address, host.c_str());
    address.port = port;

    m_peer = enet_host_connect(m_host, &address, 2, 0);
    if (!m_peer) {
        return false;
    }

    return true;
}

void ENetClient::start_service()
{
    if (m_running.load()) {
        return;
    }

    m_running.store(true);
    std::thread thread{ &ENetClient::service_thread, this };
    m_service_thread = std::move(thread);
}

void ENetClient::service_thread()
{
    ENetEvent event{};
    while (m_running.load()) {
        while (m_host && enet_host_service(m_host, &event, 8) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    on_connect(event.peer);
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    on_receive(event.peer, event.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    on_disconnect(event.peer);
                    break;
                default:
                    break;
            }
        }
    }
}
}
