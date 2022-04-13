#include <spdlog/spdlog.h>
#include <util/Variant.h>

#include <memory>

#include "server.h"
#include "../utils/textparse.h"

namespace server {
    Server::Server()
        : enetwrapper::ENetServer()
        , m_proxy_client(nullptr)
        , m_player(nullptr)
    {}

    Server::~Server() {
        delete m_proxy_client;
        delete m_player;
    }

    bool Server::initialize() {
        if (!create_host(17000, 1)) {
            delete this;
            return false;
        }

        start_service();
        return true;
    }

    void Server::on_connect(ENetPeer *peer) {
        spdlog::info("Client connected to proxy server: {}", peer->connectID);

        // Start proxy client.
        m_proxy_client = new client::Client{ this };
        if (!m_proxy_client->initialize()) {
            spdlog::error("Failed to initialize proxy client.");
        }

        m_player = new player::Player{ peer };
    }

    void Server::on_receive(ENetPeer *peer, ENetPacket *packet) {
        spdlog::info("Received packet from growtopia client: {} bytes", packet->dataLength);

        if (m_proxy_client == nullptr) {
            return;
        }

        if (m_player == nullptr) {
            return;
        }

        std::string packet_data{ player::get_text(packet) };
        if (packet_data.find("action|quit") != std::string::npos) {
            enet_peer_disconnect_later(peer, 0);
            return;
        }
    }

    void Server::on_disconnect(ENetPeer *peer) {
        spdlog::info("Client disconnected from growtopia client: (peer->data! {})", peer->data);

        delete m_player;
        m_player = nullptr;

        enet_peer_disconnect_later(m_proxy_client->get_player()->get_peer(), 0);
    }
}
