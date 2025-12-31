#pragma once
#include <httplib.h>
#include <thread>

#include "../core/config.hpp"
#include "../event/event.hpp"
#include "../network/dns_resolver.hpp"
#include "../network/client.hpp"
#include "../network/server.hpp"

namespace core {
class WebServer {
public:
    WebServer(
        Config& config,
        event::Dispatcher& dispatcher,
        network::Client& client,
        network::Server& server
    );
    ~WebServer();

private:
    void setup_server();
    void listen_internal();

    void on_client_connect(const event::Event& e);
    void on_client_disconnect(const event::Event& e);

    bool validate_response(const httplib::Result& response);

private:
    Config& config_;
    event::Dispatcher& dispatcher_;
    network::Client& client_;
    network::Server& server_;

    network::DnsResolver dns_resolver_;
    httplib::SSLServer https_server_;
    std::thread server_thread_;

    std::string pending_address_;
    uint16_t pending_port_;
};
}
