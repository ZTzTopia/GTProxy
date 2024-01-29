#pragma once
#include <vector>
#include <eventpp/callbacklist.h>
#include <spdlog/spdlog.h>

#include "config.hpp"
#include "../extension/extension.hpp"

namespace server {
class Server;
}

namespace client {
class Client;
}

namespace core {
class Core final : public extension::Extensible {
public:
    Core();
    ~Core() override;

    void run();
    void stop() { run_ = false; }

    bool add_extension(extension::IExtension* ext) override
    {
        spdlog::debug("Checking if extension with UID 0x{:x} should be ignored", ext->get_uid());
        for (const auto& ignore_uid : config_.get<std::vector<std::string>>("extension.ignore")) {
            if (ext->get_uid() == std::stoull(ignore_uid, nullptr, 16)) {
                spdlog::info("Ignoring extension with UID 0x{:x}", ext->get_uid());
                return false;
            }
        }

        return extension::Extensible::add_extension(ext);
    }

    [[nodiscard]] Config& get_config() { return config_; }
    [[nodiscard]] server::Server* get_server() const { return server_; }
    [[nodiscard]] client::Client* get_client() const { return client_; }

    [[nodiscard]] eventpp::CallbackList<void()>& get_init_callback() { return init_callback_; }
    [[nodiscard]] eventpp::CallbackList<void()>& get_tick_callback() { return tick_callback_; }

private:
    Config config_;

    server::Server* server_;
    client::Client* client_;

    bool run_;
    std::uint32_t tick_;

    eventpp::CallbackList<void()> init_callback_;
    eventpp::CallbackList<void()> tick_callback_;
};
}
