#include <string>

#include "enet_server.h"

namespace enetwrapper {
    ENetServer::ENetServer() : m_host(nullptr)
    {
        m_running.store(false);
    }

    ENetServer::~ENetServer()
    {
        destroy_host();
    }

    bool ENetServer::create_host(enet_uint16 port, std::size_t peer_count)
    {
        if (m_host) destroy_host();

        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = port;

        m_host = enet_host_create(&address, peer_count, 2, 0, 0);
        if (!m_host) return false;

		if (enet_host_compress_with_range_coder(m_host) != 0)
			return false;

        m_host->checksum = enet_crc32;
        m_host->usingNewPacketForServer = 1;
        return true;
    }

    void ENetServer::destroy_host()
    {
        if (m_running.load()) {
            m_running.store(false);
            m_service_thread.join();
        }

        if (m_host) {
            for (ENetPeer* current_peer = m_host->peers;
                current_peer < &m_host->peers[m_host->peerCount];
                ++current_peer)
            {
                if (current_peer) enet_peer_disconnect_now(current_peer, 0);
            }

            enet_host_destroy(m_host);
        }
    }

    void ENetServer::start_service()
    {
        if (m_running.load()) return;

        m_running.store(true);
        std::thread thread{ &ENetServer::service_thread, this };
        m_service_thread = std::move(thread);
    }

    void ENetServer::service_thread()
    {
        ENetEvent event;
        while (m_running.load()) {
            if (!m_host) continue;

            while (enet_host_service(m_host, &event, 8) > 0) {
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
