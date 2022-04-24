#pragma once
#include "../player/packet.h"
#include "../enetwrapper/enetserver.h"
#include "../client/client.h"
#include "../command/commandhandler.h"
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
        player::Player *get_client_player() const { return m_proxy_client->get_player(); }
        std::string get_login_info() const { return m_login_info; }

    private:
        client::Client *m_proxy_client;
        player::Player *m_player;
        command::CommandHandler *m_command_handler;
        std::string m_login_info;
    };
}
