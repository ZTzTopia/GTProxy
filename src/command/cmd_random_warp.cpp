#include "command_manager.h"
#include "../utils/random.h"

namespace command {
    void CommandManager::command_random_warp(const CommandContext& ctx)
    {
        static randutils::pcg_rng gen{ utils::random::get_generator_local() };
        std::string random_world{ utils::random::generate(gen, 16, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ") };

        CommandContext new_ctx{ ctx };
        new_ctx.args.push_back(random_world);
        command_warp(new_ctx);
    }
}