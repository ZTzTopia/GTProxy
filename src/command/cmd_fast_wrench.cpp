#include "command_manager.h"
#include "../utils/hash.h"

namespace command {
    void CommandManager::command_fast_wrench(const CommandContext& ctx)
    {
        ctx.local_player->unset_flags(player::eFlag::FAST_WRENCH_PULL);
        ctx.local_player->unset_flags(player::eFlag::FAST_WRENCH_KICK);
        ctx.local_player->unset_flags(player::eFlag::FAST_WRENCH_BAN);

        if (ctx.args.empty()) {
failed:
            ctx.server_peer->send_log(fmt::format("`4Usage: ``{}fastwrench <pull|kick|ban>", ctx.prefix));
            ctx.server_peer->send_log("Fast wrench: `4disabled``!");
            return;
        }

        switch (utils::fnv1a_hash(ctx.args[0])) {
            case "pull"_fh:
                ctx.local_player->set_flags(player::eFlag::FAST_WRENCH_PULL);
                break;
            case "kick"_fh:
                ctx.local_player->set_flags(player::eFlag::FAST_WRENCH_KICK);
                break;
            case "ban"_fh:
                ctx.local_player->set_flags(player::eFlag::FAST_WRENCH_BAN);
                break;
            default:
                goto failed;
        }

        ctx.server_peer->send_log(fmt::format("Fast wrench: `2{}``!", ctx.args[0]));
    }
}