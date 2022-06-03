#include "command_manager.h"

namespace command {
    void CommandManager::command_message_all(const CommandContext& ctx)
    {
        if (ctx.args.empty()) {
            ctx.server_peer->send_log(fmt::format("`4Usage: ``{}messageall <message>", ctx.prefix));
            return;
        }

        if (ctx.remote_players.empty()) {
            ctx.server_peer->send_log("`4Oops: ``No players in the world.");
            return;
        }

        std::string message{ ctx.args.front() };
        for (size_t i = 1; i < ctx.args.size(); ++i) {
            message += " " + ctx.args[i];
        }

        for (auto& player : ctx.remote_players) {
            ctx.client_peer->send_packet(
                player::NET_MESSAGE_GENERIC_TEXT,
                fmt::format(
                    "action|input\n"
                    "text|/msg {} {}", player.second->get_raw_name(), message));
        }
    }
}