#include "command_manager.h"

namespace command {
    void CommandManager::command_fast_drop(const CommandContext& ctx)
    {
        ctx.local_player->toggle_flags(player::eFlag::FAST_DROP);
        if (ctx.local_player->has_flags(player::eFlag::FAST_DROP))
            ctx.server_peer->send_log("Fast drop: `2enabled``!");
        else
            ctx.server_peer->send_log("Fast drop: `4disabled``!");
    }
}