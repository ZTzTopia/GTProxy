#include "command_manager.h"

namespace command {
    void CommandManager::command_nick(const CommandContext& ctx)
    {
        if (ctx.args.empty()) {
            ctx.server_peer->send_log(fmt::format("`4Usage: ``{}nickname <nickname>", ctx.prefix));
            return;
        }

        ctx.server_peer->send_variant({ "OnNameChanged", ctx.args[0] }, ctx.local_player->get_net_id());
        ctx.server_peer->send_log(fmt::format("Display name changed to {}", ctx.args[0]));
    }
}