#pragma once
#include <enet/enet.h>

#include "../core/core.hpp"
#include "../player/player.hpp"

namespace client {
class Client final {
public:
    explicit Client(core::Core* core);
    ~Client();

    [[nodiscard]] ENetPeer* connect(const std::string& host, enet_uint16 port) const;
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
