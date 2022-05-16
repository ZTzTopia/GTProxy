#pragma once
#include "../player/packet.h"
#include "../enetwrapper/enetserver.h"
#include "../client/client.h"
#include "../command/command_handler.h"
#include "../player/player.h"

namespace server {
    class Server : public enetwrapper::ENetServer {
    public:
        Server();
        ~Server();

        bool initialize();

        void on_connect(ENetPeer* peer) override;
        void on_receive(ENetPeer* peer, ENetPacket* packet) override;
        void on_disconnect(ENetPeer* peer) override;

        void process_packet(ENetPeer* peer, ENetPacket* packet);
        void process_tank_update_packet(ENetPeer* peer, player::GameUpdatePacket* game_update_packet);

        client::Client* get_client() const { return m_client; }
        player::Player* get_player() const { return m_player; }
        player::Player* get_client_player() const { return m_client->get_player(); }

    private:
        client::Client* m_client;
        player::Player* m_player;
        command::CommandHandler* m_command_handler;
    };
}
