#pragma once
#include "enet_wrapper.hpp"
#include "../core/core.hpp"
#include "../player/player.hpp"

namespace server {
class Server final : public ENetWrapper {
public:
    explicit Server(core::Core* core);
    ~Server() override;

    void process() override;

    void on_connect(ENetPeer* peer) override;
    void on_receive(ENetPeer* peer, ENetPacket* packet) override;
    void on_disconnect(ENetPeer* peer) override;

    [[nodiscard]] player::Player* get_player() const { return player_; }

private:
    core::Core* core_;
    player::Player* player_;
};
}
