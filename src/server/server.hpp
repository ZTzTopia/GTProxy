#pragma once
#include <eventpp/callbacklist.h>

#include "enet_wrapper.hpp"
#include "../core/core.hpp"
#include "../player/player.hpp"

namespace server {
class Server final : public ENetWrapper {
    using ConnectionCallback = eventpp::CallbackList<void(const player::Player&)>;
    using DisconnectionCallback = eventpp::CallbackList<void(const player::Player&)>;
    using MessageCallback = eventpp::CallbackList<void(const player::Player&, const player::Player&, const std::string&)>;

public:
    explicit Server(core::Core* core);
    ~Server() override;

    void process() override;

    void on_connect(ENetPeer* peer) override;
    void on_receive(ENetPeer* peer, ENetPacket* packet) override;
    void on_disconnect(ENetPeer* peer) override;

    [[nodiscard]] player::Player* get_player() const { return player_; }

    [[nodiscard]] ConnectionCallback& get_connect_callback() { return connect_callback_; }
    [[nodiscard]] DisconnectionCallback& get_disconnect_callback() { return disconnect_callback_; }
    [[nodiscard]] MessageCallback& get_message_callback() { return message_callback_; }

private:
    core::Core* core_;
    player::Player* player_;

    ConnectionCallback connect_callback_;
    DisconnectionCallback disconnect_callback_;
    MessageCallback message_callback_;
};
}
