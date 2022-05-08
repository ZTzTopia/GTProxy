#pragma once
#include "../enetwrapper/enetclient.h"
#include "../player/player.h"

namespace client {
    class Client : public enetwrapper::ENetClient {
    public:
        explicit Client(server::Server* server);
        ~Client();

        bool initialize();

        void on_connect(ENetPeer* peer) override;
        void on_receive(ENetPeer* peer, ENetPacket* packet) override;
        void on_disconnect(ENetPeer* peer) override;

        player::Player* get_player() const { return m_player; }
        bool is_on_send_to_server() const { return m_on_send_to_server.active; }
        std::string get_host() const { return m_on_send_to_server.host; }
        uint16_t get_port() const { return m_on_send_to_server.port; }

    private:
        server::Server* m_proxy_server;
        player::Player* m_player;

        struct {
            bool active;
            std::string host;
            enet_uint16 port;
        } m_on_send_to_server;
    };
}// namespace client
