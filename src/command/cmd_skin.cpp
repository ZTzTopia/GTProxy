#include "command_manager.h"

namespace command {
    void CommandManager::command_skin(const CommandContext& ctx)
    {
        if (ctx.args.empty()) {
            ctx.server_peer->send_log(fmt::format("`4Usage: ``{}skin [hex] <code>", ctx.prefix));
            return;
        }

        std::string lowercase{ ctx.args[0] };
        std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(), ::tolower);
        bool is_hex_code = lowercase == "hex" && std::all_of(ctx.args[1].begin(), ctx.args[1].end(), ::isxdigit);

        try {
            uint32_t skin_code{ is_hex_code ? std::stoul(ctx.args[1], nullptr, 16) : std::stoul(ctx.args[0]) };
            ctx.server_peer->send_variant({ "OnChangeSkin", skin_code }, ctx.local_player->get_net_id());
            ctx.server_peer->send_log(fmt::format("Skin changed to {}", is_hex_code ?  ctx.args[1] : ctx.args[0]));
        }
        catch (const std::exception&) {
            ctx.server_peer->send_log(fmt::format("`4Oops: ``Invalid skin code: {}", ctx.args[0]));
        }
    }
}