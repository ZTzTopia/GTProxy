#include "command_manager.h"

namespace command {
    void CommandManager::command_warp(const CommandContext& ctx)
    {
        if (ctx.args.empty()) {
            ctx.server_peer->send_log(fmt::format("`4Usage: ``{}warp <world name>", ctx.prefix));
            return;
        }

        if (ctx.args[0] == "exit") {
            ctx.server_peer->send_log("`4Oops: ``You cannot warp to the exit world.");
            return;
        }

        if (ctx.args[0].size() > 23) {
            ctx.server_peer->send_log("`4Oops: ``World name too long, try again.");
            return;
        }

        ctx.client_peer->send_packet(player::NET_MESSAGE_GAME_MESSAGE, "action|quit_to_exit");
        ctx.server_peer->send_log(fmt::format("Warping to {}...", ctx.args[0]));
        ctx.client_peer->send_packet(
            player::NET_MESSAGE_GAME_MESSAGE,
            fmt::format(
                "action|join_request\n"
                "name|{}\n"
                "invitedWorld|0", ctx.args[0]));
    }
}