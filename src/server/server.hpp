#pragma once
#include <eventpp/callbacklist.h>

#include "enet_wrapper.hpp"
#include "../core/core.hpp"
#include "../player/player.hpp"

namespace server {
class Server final : public ENetWrapper {
public:
    explicit Server(core::Core* core);
    ~Server() = default;

    void process() override;

    void on_connect(ENetPeer* peer) override;
    void on_receive(ENetPeer* peer, ENetPacket* packet) override;
    void on_disconnect(ENetPeer* peer) override;

private:
    core::Core* core_;
    player::Player* player_;

    eventpp::CallbackList<void(const player::Player&)> connect_callback_;
    eventpp::CallbackList<bool(const player::Player&, const TextParse& text_parse)> receive_message_callback_;
    eventpp::CallbackList<void(const player::Player&)> disconnect_callback_;
};
}
