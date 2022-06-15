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

        void on_connect(ENetPeer* peer) override;
        void on_receive(ENetPeer* peer, ENetPacket* packet) override;
        void on_disconnect(ENetPeer* peer) override;

        bool process_packet(ENetPeer* peer, ENetPacket* packet);

        player::Peer* get_peer() const { return m_peer; }

    private:
        Config* m_config;
        player::Peer* m_peer;
        server::Server* m_server;
    };
}
