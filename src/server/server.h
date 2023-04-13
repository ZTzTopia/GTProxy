#pragma once

#include "../http/http.h"
#include "../config.h"
#include "../enetwrapper/enet_server.h"
#include "../player/peer.h"

namespace client {
class Client;
}

namespace server {
class Server : public enet_wrapper::ENetServer {
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
    void set_gt_server_peer(player::Peer* peer) { m_peer.m_gt_server = peer; }

private:
    Config* m_config;
    server::Http* m_http;
    client::Client* m_client;

    struct {
        player::Peer* m_gt_server;
        player::Peer* m_gt_client;
    } m_peer;
};
}
