#pragma once
#include <eventpp/callbacklist.h>

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

    [[nodiscard]] eventpp::CallbackList<void()>& get_init_callback() { return init_callback_; }
    [[nodiscard]] eventpp::CallbackList<void()>& get_tick_callback() { return tick_callback_; }

    [[nodiscard]] std::tuple<std::string, uint16_t> get_address() const { return { address_, port_ }; }
    void set_address(const std::string& address, const uint16_t port) { address_ = address; port_ = port; }

private:
    Config config_;

    server::Server* server_;
    client::Client* client_;

    bool run_;
    std::uint32_t tick_;

    eventpp::CallbackList<void()> init_callback_;
    eventpp::CallbackList<void()> tick_callback_;

    std::string address_;
    uint16_t port_;
};
}
