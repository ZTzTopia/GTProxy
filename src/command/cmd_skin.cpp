#include "command_manager.h"

namespace command {
    void CommandManager::command_skin(const CommandContext& ctx)
    {
        if (ctx.args.empty()) {
            ctx.server_peer->send_log(fmt::format("`4Usage: ``{}skin <code>", ctx.prefix));
            return;
        }

        try {
            ctx.server_peer->send_variant({ "OnChangeSkin", std::stoi(ctx.args[0]) }, ctx.local_player->get_net_id());
            ctx.server_peer->send_log(fmt::format("Skin changed to {}", ctx.args[0]));
        }
        catch (const std::exception&) {
            ctx.server_peer->send_log(fmt::format("`4Oops: ``Invalid skin code: {}", ctx.args[0]));
        }
    }
}