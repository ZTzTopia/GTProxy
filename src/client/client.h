#pragma once
#include "../enetwrapper/enetclient.h"
#include "../player/player.h"

namespace client {
    class Client : public enetwrapper::ENetClient {
    public:
        explicit Client(server::Server *server);
        ~Client();

        bool initialize();

        void on_connect(ENetPeer *peer) override;
        void on_receive(ENetPeer *peer, ENetPacket *packet) override;
        void on_disconnect(ENetPeer *peer) override;

        player::Player *get_player() const { return m_player; }

    private:
        server::Server *m_proxy_server;
        player::Player *m_player;
    };
}
