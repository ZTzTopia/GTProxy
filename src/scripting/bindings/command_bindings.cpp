#include "command_bindings.hpp"
#include "../../packet/message/chat.hpp"

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
            lua_ctx["reply"] = [&ctx](const std::string& msg) -> bool {
                auto log_pkt = std::make_shared<packet::message::Log>();
                log_pkt->msg = msg;
                return packet::PacketHelper::write(*log_pkt, ctx.server);
            };

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

    cmd_table.set_function("list", [this, &lua]() {
        sol::table commands{ lua.create_table() };

        const auto& registry{ handler_.registry() };

        for (
            const auto command_list{ registry.get_all_commands() };
            const auto& [name, description] : command_list
        ) {
            commands[name] = description;
        }

        return commands;
    });

    cmd_table.set_function("execute", [this](const std::string& input) -> bool {
        return handler_.registry().execute(input, server_, client_, dispatcher_, scheduler_);
    });

    cmd_table.set_function("get_prefix", [this]() {
        return std::string{ 1, handler_.registry().prefix() };
    });

    cmd_table.set_function("set_prefix", [this](char prefix) {
        handler_.registry().set_prefix(prefix);
    });

    lua["command"] = cmd_table;
}
}
