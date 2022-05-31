#pragma once
#include "../enetwrapper/enetclient.h"
#include "../player/player.h"
#include "../player/local_player.h"
#include "../player/remote_player.h"
#include "../items/items.h"

namespace client {
    class Client : public enetwrapper::ENetClient {
    public:
        explicit Client(server::Server* server);
        ~Client();

        bool initialize();

        void on_update();
        void on_connect(ENetPeer* peer) override;
        void on_receive(ENetPeer* peer, ENetPacket* packet) override;
        void on_disconnect(ENetPeer* peer) override;

        bool process_packet(ENetPeer* peer, ENetPacket* packet);
        bool process_tank_update_packet(ENetPeer* peer, player::GameUpdatePacket* game_update_packet);

        server::Server* get_server() const { return m_server; }
        player::Player* get_player() const { return m_player; }
        player::LocalPlayer* get_local_player() const { return m_local_player; }
        std::unordered_map<uint32_t, player::RemotePlayer*> get_remote_players() const { return m_remote_player; }
        bool is_on_send_to_server() const { return m_on_send_to_server.active; }
        std::string get_host() const { return m_on_send_to_server.host; }
        uint16_t get_port() const { return m_on_send_to_server.port; }

        bool is_disconnected() const { return m_disconnected; }

    private:
        server::Server* m_server;
        player::Player* m_player;
        items::Items* m_items;
        player::LocalPlayer* m_local_player;
        std::unordered_map<uint32_t, player::RemotePlayer*> m_remote_player;

        bool m_disconnected;

        struct {
            bool active;
            std::string host;
            enet_uint16 port;
        } m_on_send_to_server;

        std::atomic<bool> m_on_update_thread_running;
        std::thread m_on_update_thread;
    };
}// namespace client
