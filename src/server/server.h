#pragma once
#include "../player/packet.h"
#include "../enetwrapper/enetserver.h"
#include "../client/client.h"
#include "../player/player.h"

namespace server {
    class Server : public enetwrapper::ENetServer {
    public:
        Server();
        ~Server();

        bool initialize();

        void on_connect(ENetPeer *peer) override;
        void on_receive(ENetPeer *peer, ENetPacket *packet) override;
        void on_disconnect(ENetPeer *peer) override;

        player::Player *get_player() const { return m_player; }

    private:
        client::Client *m_proxy_client;
        player::Player *m_player;
    };
}
