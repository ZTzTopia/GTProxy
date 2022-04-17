#include <string>

#include "enetserver.h"

namespace enetwrapper {
    ENetServer::ENetServer()
        : m_host(nullptr)
    {
        m_running.store(false);
    }

    ENetServer::~ENetServer() {
        if (m_running.load()) {
            m_running.store(false);
            m_service_thread.join();
        }

        if (m_host) {
            enet_host_destroy(m_host);
            for (ENetPeer *current_peer = m_host->peers; current_peer < &m_host->peers[m_host->peerCount]; ++current_peer) {
                enet_peer_disconnect(current_peer, 0);
            }
        }
    }

    bool ENetServer::create_host(enet_uint16 port, size_t peer_count) {
        if (m_host) {
			enet_host_destroy(m_host);
			m_host = nullptr;
		}

        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = port;

        m_host = enet_host_create(&address, peer_count, 2, 0, 0);
        if (m_host == nullptr) {
            return false;
        }

        m_host->checksum = enet_crc32;
        // m_host->duplicatePeers = 3; // 3 peers are allowed to connect from the same IP address.
        // m_host->usingNewPacket = 1;
        m_host->usingNewPacketForServer = 1;
		if (enet_host_compress_with_range_coder(m_host) != 0) {
			return false;
		}

        return true;
    }

    void ENetServer::start_service() {
        if (m_running.load()) {
            return;
        }

        m_running.store(true);
        std::thread thread{ &ENetServer::service_thread, this };
        m_service_thread = std::move(thread);
    }

    void ENetServer::service_thread() {
        ENetEvent event;
        while (m_running.load()) {
            if (m_host == nullptr) {
                continue;
            }

            if (enet_host_service(m_host, &event, 32) > 0) { // Please don't use timeout 0. or your pc will be a bomb.
                switch (event.type) {
                    case ENET_EVENT_TYPE_CONNECT:
                        on_connect(event.peer);
                        break;
                    case ENET_EVENT_TYPE_RECEIVE:
                        on_receive(event.peer, event.packet);
                        // enet_packet_destroy(event.packet);
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
