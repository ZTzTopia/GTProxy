#pragma once
#include "../enetwrapper/enetclient.h"
#include "../player/player.h"

namespace client {
    class Client : public enetwrapper::ENetClient {
        struct SendServerInfo {
            uint32_t port;
            uint32_t token;
            uint32_t user;
            std::string host;
            std::string uuid_token;
            bool check;
        };
    public:
        explicit Client(server::Server *server);
        ~Client();

        bool initialize();

        void on_connect(ENetPeer *peer) override;
        void on_receive(ENetPeer *peer, ENetPacket *packet) override;
        void on_disconnect(ENetPeer *peer) override;

        player::Player *get_player() const { return m_player; }
        SendServerInfo *get_send_server_info() const { return m_send_server_info; }

    private:
        server::Server *m_proxy_server;
        player::Player *m_player;
        SendServerInfo *m_send_server_info;
    };
}
