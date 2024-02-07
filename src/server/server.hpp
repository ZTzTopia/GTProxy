#pragma once
#include <enet/enet.h>

#include "../core/core.hpp"
#include "../player/player.hpp"

namespace server {
class Server final {
public:
    explicit Server(core::Core* core);
    ~Server();

    void process();

    void on_connect(ENetPeer* peer);
    void on_receive(ENetPeer* peer, ENetPacket* packet);
    void on_disconnect(ENetPeer* peer);

    [[nodiscard]] player::Player* get_player() const { return player_; }

private:
    ENetHost* host_;
    core::Core* core_;
    player::Player* player_;
};
}
