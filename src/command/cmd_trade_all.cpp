#include "command_manager.h"

namespace command {
    void CommandManager::command_trade_all(const CommandContext& ctx)
    {
        if (ctx.local_player->get_user_id() != ctx.local_player->get_world()->world_owner_id) {
            ctx.server_peer->send_log("`4Oops: ``You are not the owner of this world!");
            return;
        }

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