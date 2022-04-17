#include <string>

#include "enetclient.h"

namespace enetwrapper {
    ENetClient::ENetClient()
        : m_host(nullptr)
        , m_peer(nullptr)
    {
        srand(static_cast<unsigned int>(time(nullptr)) + clock());
        m_running.store(false);
    }

    ENetClient::~ENetClient() {
        if (m_running.load()) {
            m_running.store(false);
            m_service_thread.join();
        }

        if (m_host) {
            enet_host_destroy(m_host);
            if (m_peer && m_peer->state == ENET_PEER_STATE_CONNECTED) {
                enet_peer_disconnect(m_peer, 0);
            }
        }
    }

    bool ENetClient::connect(const std::string &host, enet_uint16 port, size_t peer_count) {
        if (m_host) {
			enet_host_destroy(m_host);
			m_host = nullptr;
		}

        ENetAddress address;
        enet_address_set_host(&address, host.c_str());
        address.port = port;

        m_host = enet_host_create(nullptr, peer_count, 2, 0, 0);
        if (m_host == nullptr) {
            return false;
        }

        m_peer = enet_host_connect(m_host, &address, 2, 0);
        if (m_peer == nullptr) {
            return false;
        }

        m_host->checksum = enet_crc32;
        // m_host->duplicatePeers = 3; // 3 peers are allowed to connect from the same IP address.
        m_host->usingNewPacket = 1;
        // m_host->usingNewPacketForServer = 1;
		if (enet_host_compress_with_range_coder(m_host) != 0) {
			return false;
		}

        return true;
    }

    void ENetClient::start_service() {
        if (m_running.load()) {
            return;
        }

        m_running.store(true);
        std::thread thread{ &ENetClient::service_thread, this };
        m_service_thread = std::move(thread);
    }

    void ENetClient::service_thread() {
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
