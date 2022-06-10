#include "command_manager.h"

namespace command {
    void CommandManager::command_nick(const CommandContext& ctx)
    {
        if (ctx.args.empty()) {
            ctx.server_peer->send_log(fmt::format("`4Usage: ``{}nickname <nickname>", ctx.prefix));
            return;
        }
        std::string name{ ctx.args.front() };
        for (size_t i = 1; i < ctx.args.size(); ++i) {
            name += " " + ctx.args[i];
        }
        ctx.server_peer->send_variant({ "OnNameChanged", name }, ctx.local_player->get_net_id());
        ctx.server_peer->send_log(fmt::format("Display name changed to {}", name));
    }
}
