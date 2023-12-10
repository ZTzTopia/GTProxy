#pragma once
#include "config.hpp"

namespace server {
class Server;
}

namespace client {
class Client;
}

namespace core {
class Core {
public:
    Core();
    ~Core();

    void run();
    void stop() { run_ = false; }

    [[nodiscard]] Config& get_config() { return config_; }
    [[nodiscard]] server::Server* get_server() const { return server_; }
    [[nodiscard]] client::Client* get_client() const { return client_; }

private:
    Config config_;

    server::Server* server_;
    client::Client* client_;

    bool run_;
    std::uint32_t tick_;
};
}
