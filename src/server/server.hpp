#pragma once
#include <eventpp/callbacklist.h>

#include "enet_wrapper.hpp"
#include "web_server.hpp"
#include "../core/core.hpp"
#include "../player/player.hpp"
#include "../utils/text_parse.hpp"

namespace server {
class Server final : public ENetWrapper {
public:
    explicit Server(core::Core* core);
    ~Server() override;

    void process() override;

    void on_connect(ENetPeer* peer) override;
    void on_receive(ENetPeer* peer, ENetPacket* packet) override;
    void on_disconnect(ENetPeer* peer) override;

    [[nodiscard]] WebServer& get_web_server() { return web_server_; }
    [[nodiscard]] player::Player* get_player() const { return player_; }

    [[nodiscard]] eventpp::CallbackList<void(const player::Player&)>& get_connect_callback() { return connect_callback_; }
    [[nodiscard]] eventpp::CallbackList<void(const player::Player&)>& get_disconnect_callback() { return disconnect_callback_; }
    [[nodiscard]] eventpp::CallbackList<bool(const player::Player&, const TextParse&)>& get_receive_message_callback() { return receive_message_callback_; }

private:
    core::Core* core_;
    WebServer web_server_;
    player::Player* player_;

    eventpp::CallbackList<void(const player::Player&)> connect_callback_;
    eventpp::CallbackList<void(const player::Player&)> disconnect_callback_;
    eventpp::CallbackList<bool(const player::Player&, const TextParse&)> receive_message_callback_;
};
}
