#include "command_manager.h"

namespace command {
    void CommandManager::command_fast_trash(const CommandContext& ctx)
    {
        ctx.local_player->toggle_flags(player::eFlag::FAST_TRASH);
        if (ctx.local_player->has_flags(player::eFlag::FAST_TRASH))
            ctx.server_peer->send_log("Fast trash: `2enabled``!");
        else
            ctx.server_peer->send_log("Fast trash: `4disabled``!");
    }
}