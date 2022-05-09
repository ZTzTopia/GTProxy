#include <string>

#include "enetclient.h"
#include "../config.h"

namespace enetwrapper {
    ENetClient::ENetClient() : m_host(nullptr), m_peer(nullptr)
    {
        m_running.store(false);
    }

    ENetClient::~ENetClient()
    {
        destroy();
    }

    bool ENetClient::create_host(std::size_t peer_count) {
        if (m_host) destroy();

        m_host = enet_host_create(nullptr, peer_count, 2, 0, 0);
        if (!m_host) return false;

        if (enet_host_compress_with_range_coder(m_host) != 0)
			return false;

        m_host->checksum = enet_crc32;
        m_host->usingNewPacket = Config::get().config()["server"]["usingNewPacket"].get<enet_uint8>();
        return true;
    }

    void ENetClient::destroy()
    {
        if (m_running.load()) {
            m_running.store(false);
            m_service_thread.join();
        }

        if (m_host) {
            enet_host_destroy(m_host);
            if (m_peer && m_peer->state == ENET_PEER_STATE_CONNECTED)
                enet_peer_disconnect_now(m_peer, 0);
        }
    }

    bool ENetClient::connect(const std::string &host, enet_uint16 port)
    {
        if (!m_host) return false;

        ENetAddress address;
        enet_address_set_host(&address, host.c_str());
        address.port = port;

        m_peer = enet_host_connect(m_host, &address, 2, 0);
        if (!m_peer) return false;
        return true;
    }

    void ENetClient::start_service() {
        if (m_running.load()) return;

        m_running.store(true);
        std::thread thread{ &ENetClient::service_thread, this };
        m_service_thread = std::move(thread);
    }

    void ENetClient::service_thread()
    {
        ENetEvent event;
        while (m_running.load()) {
            if (!m_host) continue;

            if (enet_host_service(m_host, &event, 100) > 0) {
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
