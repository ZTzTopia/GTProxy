#pragma once
#include "command_registry.hpp"
#include "../core/config.hpp"
#include "../core/scheduler.hpp"
#include "../event/event.hpp"
#include "../network/client.hpp"
#include "../network/server.hpp"

namespace command {
class CommandHandler {
public:
    CommandHandler(
        core::Config& config,
        event::Dispatcher& dispatcher,
        std::shared_ptr<core::Scheduler> scheduler,
        network::Server& server,
        network::Client& client
    );
    ~CommandHandler();

    CommandRegistry& registry() { return registry_; }
    const CommandRegistry& registry() const { return registry_; }

private:
    void register_default_commands();

    void on_text_packet(const event::Event& e);

private:
    core::Config& config_;
    event::Dispatcher& dispatcher_;
    std::shared_ptr<core::Scheduler> scheduler_;
    network::Server& server_;
    network::Client& client_;

    CommandRegistry registry_;
    event::Dispatcher::Handle listener_handle_;
};
}
