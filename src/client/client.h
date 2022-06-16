#pragma once
#include "../config.h"
#include "../enetwrapper/enet_client.h"
#include "../player/peer.h"

namespace server {
    class Server;
}

namespace client {
    class Client : public enetwrapper::ENetClient {
    public:
        Client(Config* config, server::Server* server);
        ~Client();

        bool start(const std::string& host, enet_uint16 port);
        bool redirect() { return start(m_redirect_host, m_redirect_port); }

        void on_connect(ENetPeer* peer) override;
        void on_receive(ENetPeer* peer, ENetPacket* packet) override;
        void on_disconnect(ENetPeer* peer) override;

        bool process_packet(ENetPeer* peer, ENetPacket* packet);
        bool process_tank_update_packet(ENetPeer* peer, player::GameUpdatePacket* game_update_packet);

    public:
        player::Peer* get_peer() const { return m_peer; }
        bool is_redirecting() const { return m_redirecting; }

    private:
        Config* m_config;
        player::Peer* m_peer;
        server::Server* m_server;

        bool m_redirecting;
        std::string m_redirect_host;
        enet_uint16 m_redirect_port;
    };
}
