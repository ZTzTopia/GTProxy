#pragma once
#include "config.hpp"
#include "../event/event.hpp"
#include "../network/client.hpp"
#include "../network/server.hpp"

namespace core {
class SessionHandler {
public:
    explicit SessionHandler(
        Config& config,
        event::Dispatcher& dispatcher,
        network::Client& client,
        network::Server& server
    );
    ~SessionHandler();

private:
    Config& config_;
    event::Dispatcher& dispatcher_;
    network::Client& client_;
    network::Server& server_;

    std::string pending_address_;
    uint16_t pending_port_;

    void setup_raw_packet_handlers() const;
    void setup_unknown_packet_forwarders() const;
    void setup_on_send_to_server_handler();
    void setup_quit_handler() const;
    void setup_disconnect_handler() const;
    void setup_connection_handlers();
};
}
