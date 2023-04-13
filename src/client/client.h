#pragma once

#include "../config.h"
#include "../enetwrapper/enet_client.h"
#include "../player/peer.h"

namespace server {
class Server;
class Http;
}

namespace client {
class Client : public enet_wrapper::ENetClient {
public:
    Client(Config* config, server::Http* http, server::Server* server);
    ~Client();

    void start();

    void on_connect(ENetPeer* peer) override;
    void on_receive(ENetPeer* peer, ENetPacket* packet) override;
    void on_disconnect(ENetPeer* peer) override;

    bool process_packet(ENetPeer* peer, ENetPacket* packet);
    bool process_tank_update_packet(ENetPeer* peer, player::GameUpdatePacket* game_update_packet);

public:
    void set_gt_client_peer(player::Peer* peer) { m_peer.m_gt_client = peer; }

private:
    Config* m_config;
    server::Http* m_http;
    server::Server* m_server;

    struct {
        player::Peer* m_gt_server;
        player::Peer* m_gt_client;
    } m_peer;

    struct {
        enet_uint8 m_using_new_packet;
        std::string m_host;
        enet_uint16 m_port;
    } m_redirect_server;
};
}
