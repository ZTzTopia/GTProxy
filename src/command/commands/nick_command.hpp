#pragma once
#include <string>
#include <fmt/format.h>

#include "../command.hpp"
#include "../command_registry.hpp"
#include "../../packet/packet_helper.hpp"
#include "../../packet/game/player.hpp"
#include "../../packet/message/chat.hpp"
#include "../../world/world.hpp"

namespace command {
class NickCommand final : public ICommand {
public:
    [[nodiscard]] std::string_view name() const override { return "nick"; }
    [[nodiscard]] std::string description() const override { return "Change your display name"; }

    Result execute(const Context& ctx) override
    {
        if (ctx.args.empty()) {
            packet::message::Log log{};
            log.msg = fmt::format("`4Usage: ``{}nick <nickname>", ctx.registry.prefix());
            packet::PacketHelper::write(log, ctx.server);
            return Result::InvalidArguments;
        }

        std::string name{ ctx.args.front() };
        for (size_t i{ 1 }; i < ctx.args.size(); ++i) {
            name += ' ' + ctx.args[i];
        }

        packet::game::OnNameChanged pkt{};
        pkt.net_id = world::World::instance().get_local_net_id();
        pkt.name = name;
        packet::PacketHelper::write(pkt, ctx.server);

        packet::message::Log log{};
        log.msg = fmt::format("Display name changed to {}", name);
        packet::PacketHelper::write(log, ctx.server);

        return Result::Success;
    }
};
}
