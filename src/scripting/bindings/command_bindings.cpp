#include "command_bindings.hpp"

namespace scripting::bindings {
void CommandBindings::bind(sol::state& lua)
{
    auto cmd_table{ lua.create_table() };

    auto register_impl = [this, &lua](
        const std::string& name,
        const std::string& description,
        sol::protected_function callback
    ) {
        handler_.registry().add(name, description, [callback, &lua, name](const command::Context& ctx) -> command::Result {
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
                    const sol::error err = result;
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
    };

    cmd_table.set_function("register", sol::overload(
        register_impl,
        [register_impl](const std::string& name, sol::protected_function callback) {
            return register_impl(name, "Script command", callback);
        }
    ));

    cmd_table.set_function("prefix", [this]() {
        return std::string{ 1, handler_.registry().prefix() };
    });

    lua["command"] = cmd_table;
}
}
