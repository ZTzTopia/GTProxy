#include <spdlog/spdlog.h>
#include <util/Variant.h>

#include <memory>

#include "client.h"
#include "../server/server.h"
#include "../utils/textparse.h"

namespace client {
    Client::Client(server::Server *server)
        : enetwrapper::ENetClient()
        , m_proxy_server(server)
        , m_player(nullptr)
    {}

    Client::~Client() {
        delete m_proxy_server;
        delete m_player;
    }

    bool Client::initialize() {
        if (!connect("213.179.209.168", 17253, 1)) {
            delete this;
            return false;
        }

        start_service();
        return true;
    }

    void Client::on_connect(ENetPeer *peer) {
        spdlog::info("Client connected to growtopia server: {}", peer->connectID);
        m_player = new player::Player{ peer };
    }

    void Client::on_receive(ENetPeer *peer, ENetPacket *packet) {
        spdlog::info("Received packet from growtopia server: {} bytes", packet->dataLength);

        if (m_proxy_server == nullptr) {
            return;
        }

        if (m_player == nullptr) {
            return;
        }

        m_proxy_server->get_player()->send_packet_packet(packet);
    }

    void Client::on_disconnect(ENetPeer *peer) {
        spdlog::info("Client disconnected from growtopia server: (peer->data! {})", peer->data);

        delete m_player;
        delete this;

        if (m_proxy_server->get_player()) {
            enet_peer_disconnect_later(m_proxy_server->get_player()->get_peer(), 0);
        }
    }
}
