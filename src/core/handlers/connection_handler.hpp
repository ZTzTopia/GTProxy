#pragma once
#include <string>
#include <vector>

#include "../../event/event.hpp"
#include "../../network/client.hpp"
#include "../../network/server.hpp"
#include "../../utils/hash.hpp"
#include "../config.hpp"

namespace core::handlers {
class ConnectionHandler {
public:
    ConnectionHandler(
        event::Dispatcher& dispatcher,
        network::Client& client,
        network::Server& server,
        Config& config
    );

private:
    void setup_connection_handlers();
    void setup_on_send_to_server_handler();
    void setup_quit_handler();
    void setup_disconnect_handler();
    void setup_send_item_database_data_handler();
    void setup_on_super_main_start_handler();

private:
    event::Dispatcher& dispatcher_;
    network::Client& client_;
    network::Server& server_;
    Config& config_;

    std::string pending_address_;
    uint16_t pending_port_;

    std::vector<event::ScopedHandle> handles_;
};
}
