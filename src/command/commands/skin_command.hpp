#pragma once
#include <algorithm>
#include <string>
#include <fmt/format.h>

#include "../command.hpp"
#include "../command_registry.hpp"
#include "../../packet/packet_helper.hpp"
#include "../../packet/game/player.hpp"
#include "../../packet/message/chat.hpp"
#include "../../world/world.hpp"

namespace command {
class SkinCommand final : public ICommand {
public:
    [[nodiscard]] std::string_view name() const override { return "skin"; }
    [[nodiscard]] std::string description() const override { return "Change your skin code"; }

    Result execute(const Context& ctx) override
    {
        if (ctx.args.empty()) {
            packet::message::Log log{};
            log.msg = fmt::format("`4Usage: ``{}skin [hex] <code>", ctx.registry.prefix());
            packet::PacketHelper::write(log, ctx.server);
            return Result::InvalidArguments;
        }

        std::string lowercase{ ctx.args[0] };
        std::ranges::transform(lowercase, lowercase.begin(), ::tolower);

        bool is_hex_code = false;
        if (lowercase == "hex" && ctx.args.size() > 1) {
            is_hex_code = std::ranges::all_of(ctx.args[1], ::isxdigit);
        }

        uint32_t skin_code{};
        if (
            auto [ptr, ec]{
                std::from_chars(
                is_hex_code ? ctx.args[1].data() : ctx.args[0].data(),
                is_hex_code ? ctx.args[1].data() + ctx.args[1].size() : ctx.args[0].data() + ctx.args[0].size(),
                skin_code,
                is_hex_code ? 16 : 10)
            };
            ec != std::errc{}
        ) {
            packet::message::Log log{};
            log.msg = fmt::format("`4Oops: ``Invalid skin code: {}", is_hex_code ? ctx.args[1] : ctx.args[0]);
            packet::PacketHelper::write(log, ctx.server);
            return Result::Failed;
        }

        packet::game::OnChangeSkin pkt{};
        pkt.net_id = world::World::instance().get_local_net_id();
        pkt.skin_code = skin_code;
        packet::PacketHelper::write(pkt, ctx.server);

        packet::message::Log log{};
        log.msg = fmt::format("Skin changed to {}", is_hex_code ? ctx.args[1] : ctx.args[0]);
        packet::PacketHelper::write(log, ctx.server);

        return Result::Success;
    }
};
}
