#pragma once
#include <vector>

#include "../../event/event.hpp"
#include "../../network/client.hpp"
#include "../../network/server.hpp"

namespace core::handlers {
class ForwardingHandler {
public:
    ForwardingHandler(
        event::Dispatcher& dispatcher,
        network::Client& client,
        network::Server& server
    );

private:
    void setup_raw_packet_handlers();

private:
    event::Dispatcher& dispatcher_;
    network::Client& client_;
    network::Server& server_;

    std::vector<event::ScopedHandle> handles_;
};
}
