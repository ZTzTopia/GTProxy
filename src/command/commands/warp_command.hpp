#pragma once
#include "../command.hpp"
#include "../../packet/packet_helper.hpp"
#include "../../packet/message/chat.hpp"
#include "../../packet/message/exit.hpp"

namespace command {
class WarpCommand final : public ICommand {
public:
    [[nodiscard]] std::string_view name() const override { return "warp"; }
    [[nodiscard]] std::string description() const override { return "Warp to a world."; }

    Result execute(const Context& ctx) override
    {
        if (ctx.args.empty()) {
            packet::message::Log pkt{};
            pkt.msg = fmt::format("`4Usage: ``{}warp <world name>", ctx.registry.prefix());
            packet::PacketHelper::write(pkt, ctx.server);
            return Result::InvalidArguments;
        }

        const std::string world_name{ ctx.args[0] };

        if (world_name == "exit") {
            packet::message::Log pkt{};
            pkt.msg = "`4Oops: ``You cannot warp to the exit world.";
            packet::PacketHelper::write(pkt, ctx.server);
            return Result::InvalidArguments;
        }

        if (world_name.size() > 23) {
            packet::message::Log pkt{};
            pkt.msg = "`4Error: ``World name cannot exceed 23 characters.";
            packet::PacketHelper::write(pkt, ctx.server);
            return Result::InvalidArguments;
        }

        ctx.scheduler.cancel_by_tag("warp");

        packet::message::QuitToExit quit_pkt{};
        packet::PacketHelper::write(quit_pkt, ctx.client);

        packet::message::Log log{};
        log.msg = fmt::format("Warping to {}...", world_name);
        packet::PacketHelper::write(log, ctx.server);

        const auto client{ &ctx.client };
        ctx.scheduler.schedule_delayed(
            [client, &world_name] {
                if (!client->is_connected()) {
                    spdlog::warn("Client disconnected before warp could complete.");
                    return;
                }

                packet::message::JoinRequest join_pkt{};
                join_pkt.world_name = world_name;
                join_pkt.invited_world = false;
                packet::PacketHelper::write(join_pkt, *client);
            },
            std::chrono::milliseconds{ 1750 },
            "warp",
            core::TaskPriority::Normal
        );

        return Result::Success;
    }
};
}
