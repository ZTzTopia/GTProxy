#pragma once
#include "http.h"
#include "../config.h"
#include "../enetwrapper/enet_server.h"
#include "../player/peer.h"

namespace client {
    class Client;
}

namespace server {
    class Server : public enetwrapper::ENetServer {
    public:
        explicit Server(Config* config);
        ~Server();

        bool start();

        void on_connect(ENetPeer* peer) override;
        void on_receive(ENetPeer* peer, ENetPacket* packet) override;
        void on_disconnect(ENetPeer* peer) override;

        bool process_packet(ENetPeer* peer, ENetPacket* packet);
        bool process_tank_update_packet(ENetPeer* peer, player::GameUpdatePacket* game_update_packet);

    public:
        player::Peer* get_peer() const { return m_peer; }

    private:
        Config* m_config;
        server::Http* m_http;
        player::Peer* m_peer;
        client::Client* m_client;
    };
}
