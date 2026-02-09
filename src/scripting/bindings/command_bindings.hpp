#pragma once
#include <memory>

#include <sol/sol.hpp>

#include "../binding_module.hpp"
#include "../../command/command_handler.hpp"
#include "../../core/scheduler.hpp"
#include "../../event/event.hpp"
#include "../../network/client.hpp"
#include "../../network/server.hpp"

namespace scripting::bindings {
class CommandBindings final : public IBindingModule {
 public:
    explicit CommandBindings(
        command::CommandHandler& handler,
        network::Server& server,
        network::Client& client,
        event::Dispatcher& dispatcher,
        std::shared_ptr<core::Scheduler> scheduler
    )
        : handler_{ handler }
        , server_{ server }
        , client_{ client }
        , dispatcher_{ dispatcher }
        , scheduler_{ std::move(scheduler) }
    {

    }

    [[nodiscard]] std::string_view name() const override { return "command"; }

    void bind(sol::state& lua) override;

 private:
    command::CommandHandler& handler_;
    network::Server& server_;
    network::Client& client_;
    event::Dispatcher& dispatcher_;
    std::shared_ptr<core::Scheduler> scheduler_;
};
}
