#include "command_manager.h"

namespace command {
    void CommandManager::command_help(const CommandContext& ctx)
    {
        if (!ctx.args.empty()) {
            auto it = std::find_if(m_commands.begin(), m_commands.end(), [&](const auto& command) {
                if (command.second->get_name() != ctx.args[0]) {
                    std::vector<std::string> aliases = command.second->get_aliases();
                    return std::find(aliases.cbegin(), aliases.cend(), ctx.args[0]) != aliases.cend();
                }

                return command.second->get_name() == ctx.args[0];
            });

            if (it != m_commands.end())
                ctx.server_peer->send_log((*it).second->get_description());
            else
                ctx.server_peer->send_log("`4Unknown command. ``Enter `$!help`` for a list of valid commands.");

            return;
        }

        std::string commands{ ">> Commands: " };
        for (auto &command : m_commands) {
            commands.append(ctx.prefix);
            commands.append(command.second->get_name());
            commands.push_back(' ');
        }

        commands.pop_back();
        ctx.server_peer->send_log(commands);
    }
}