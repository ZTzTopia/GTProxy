#pragma once
#include "config.hpp"
#include "session_handler.hpp"
#include "web_server.hpp"
#include "../network/server.hpp"
#include "../network/client.hpp"

namespace core {
class Core final {
public:
    Core();
    ~Core();

    Core(const Core&) = delete;
    Core& operator=(const Core&) = delete;
    Core(Core&&) = delete;
    Core& operator=(Core&&) = delete;

    void run() const;
    void stop() { running_ = false; }

    [[nodiscard]] Config& get_config() { return config_; }

private:
    Config config_;
    bool running_;

    event::Dispatcher dispatcher_;

    std::unique_ptr<WebServer> web_server_;
    std::unique_ptr<network::Server> server_;
    std::unique_ptr<network::Client> client_;

    std::unique_ptr<SessionHandler> session_handler_;
};
}
