#pragma once
#include <vector>

#include <sol/sol.hpp>
#include <spdlog/spdlog.h>

#include "../binding_module.hpp"
#include "../../command/command_handler.hpp"

namespace scripting::bindings {
class CommandBindings final : public IBindingModule {
public:
    explicit CommandBindings(
        command::CommandHandler& handler
    )
        : handler_{ handler }
    {

    }

    [[nodiscard]] std::string_view name() const override { return "command"; }

    void bind(sol::state& lua) override;

private:
    command::CommandHandler& handler_;
};
}
