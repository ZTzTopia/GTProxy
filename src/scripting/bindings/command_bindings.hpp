#pragma once
#include <vector>

#define SOL_ALL_SAFETIES_ON 1
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

    void bind(sol::state& lua) override
    {
        auto cmd_table{ lua.create_table() };

        cmd_table.set_function("register", [this, &lua](
            const std::string& name,
            sol::protected_function callback
        ) {
            handler_.registry().add(name, [callback, &lua, name](const command::Context& ctx) -> command::Result {
                sol::table args_table{ lua.create_table() };
                for (size_t i{ 0 }; i < ctx.args.size(); ++i) {
                    args_table[i + 1] = ctx.args[i];
                }

                sol::table lua_ctx{ lua.create_table() };
                lua_ctx["args"] = args_table;
                lua_ctx["raw"] = ctx.raw_input;

                try {
                    const sol::protected_function_result result{ callback(lua_ctx) };

                    if (!result.valid()) {
                        const sol::error err{ result };
                        spdlog::error("[Lua Command] Error in '{}': {}", name, err.what());
                        return command::Result::Failed;
                    }

                    if (result.get_type() == sol::type::boolean) {
                        return result.get<bool>() ? command::Result::Success : command::Result::Failed;
                    }
                }
                catch (const std::exception& e) {
                    spdlog::error("[Lua Command] Exception in '{}': {}", name, e.what());
                    return command::Result::Failed;
                }
                catch (...) {
                    spdlog::error("[Lua Command] Unknown exception in '{}'", name);
                    return command::Result::Failed;
                }

                return command::Result::Success;
            });

            spdlog::info("[Lua] Registered command: /{}", name);
            return true;
        });

        cmd_table.set_function("prefix", [this]() {
            return std::string{ 1, handler_.registry().prefix() };
        });

        lua["command"] = cmd_table;
    }

private:
    command::CommandHandler& handler_;
};
}
