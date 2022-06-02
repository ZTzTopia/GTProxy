#include "command_manager.h"
#include "../utils/random.h"

namespace command {
    void CommandManager::command_auto_collect(const CommandContext& ctx)
    {
        ctx.local_player->unset_flags(player::AUTO_COLLECT);

        if (ctx.args.empty()) {
            ctx.server_peer->send_log(fmt::format("`4Usage: ``{}autocollect <radius>", ctx.prefix));
            ctx.server_peer->send_log("Auto collect: `4disabled``!");
            return;
        }

        uint8_t radius{ static_cast<uint8_t>(std::stoi(ctx.args[0])) };
        /*if (radius > 5) {
            ctx.server_peer->send_log("`4Oops: ``Radius must be less than 5!");
            return;
        }*/

        ctx.local_player->set_flags(player::AUTO_COLLECT);
        ctx.local_player->m_auto_collect_radius = radius;
        ctx.server_peer->send_log(fmt::format("Auto collect radius: `2{}``!", radius));
    }
}