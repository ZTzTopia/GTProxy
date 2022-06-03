#include "command_manager.h"

namespace command {
    void CommandManager::command_trade_all(const CommandContext& ctx)
    {
        if (ctx.remote_players.empty()) {
            ctx.server_peer->send_log("`4Oops: ``No players in the world.");
            return;
        }

        for (auto& player : ctx.remote_players) {
            ctx.client_peer->send_packet(
                player::NET_MESSAGE_GENERIC_TEXT,
                fmt::format(
                    "action|input\n"
                    "text|/trade {}", player.second->get_raw_name()));
        }
    }
}