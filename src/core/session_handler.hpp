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
    Config& config;
    event::Dispatcher& dispatcher_;
    network::Client& client_;
    network::Server& server_;
};
}
