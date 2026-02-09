#pragma once
#include <charconv>
#include <string>
#include <vector>
#include <fmt/format.h>

#include "../command.hpp"
#include "../../packet/packet_helper.hpp"
#include "../../packet/message/chat.hpp"
#include "../../player/player.hpp"
#include "../../world/object_map.hpp"
#include "../../world/tile_map.hpp"
#include "../../world/world.hpp"

namespace command {
class DebugCommand final : public ICommand {
public:
    [[nodiscard]] std::string_view name() const override { return "debug"; }
    [[nodiscard]] std::string description() const override { return "Show debug information about player, world, tile, or object"; }

    Result execute(const Context& ctx) override
    {
        if (ctx.args.empty()) {
            send_log(ctx, "Usage: /debug <player|world|tile|object>");
            return Result::Failed;
        }

        const std::string& sub_cmd{ ctx.args[0] };

        if (sub_cmd == "player") {
            const auto& world{ world::World::instance() };
            const auto& players = world.get_players();

            if (ctx.args.size() < 2) {
                send_log(ctx, fmt::format("Total Players: {}", players.size()));
                for (const auto& [net_id, player] : players) {
                    send_log(
                        ctx,
                        fmt::format(
                            "[{}] {} ({}, {})",
                            net_id,
                            player->name(),
                            player->position().x,
                            player->position().y
                        )
                    );
                }

                return Result::Success;
            }

            int32_t net_id{ 0 };
            const auto& arg{ ctx.args[1] };
            if (
                auto [ptr, ec]{ std::from_chars(arg.data(), arg.data() + arg.size(), net_id) };
                ec != std::errc{}
            ) {
                send_log(ctx, "Invalid NetID format.");
                return Result::Failed;
            }

            if (const auto player{ world.get_player(net_id).lock() }) {
                send_log(ctx, fmt::format("Player Info (NetID: {}):", net_id));
                send_log(ctx, fmt::format("Name: {} (User: {})", player->name(), player->user_id()));
                send_log(ctx, fmt::format("Pos: {}, {}", player->position().x, player->position().y));
                send_log(ctx, fmt::format("Country: {}", player->country_code()));
                send_log(ctx, fmt::format("Mod: {}, SuperMod: {}", player->mod_state(), player->supermod_state()));
                return Result::Success;
            }

            send_log(ctx, fmt::format("Player with NetID {} not found.", net_id));
            return Result::Failed;
        }

        if (sub_cmd == "world") {
            auto& world{ world::World::instance() };
            auto& tile_map{ world.get_tile_map() };
            auto& object_map{ world.get_object_map() };

            send_log(ctx, "World Info:");
            send_log(ctx, fmt::format("Version: {}", world.get_version()));
            send_log(ctx, fmt::format("Size: {}x{}", tile_map.get_size().x, tile_map.get_size().y));
            send_log(ctx, fmt::format("Tiles: {}", tile_map.get_tiles().size()));
            send_log(ctx, fmt::format("Objects: {}", object_map.get_objects().size()));
            send_log(ctx, fmt::format("Player Count: {}", world.get_players().size()));
            return Result::Success;
        }

        send_log(ctx, fmt::format("Unknown debug target: {}", sub_cmd));
        return Result::Failed;
    }

private:
    // Helper to send log messages to the player (TODO: move to a common utility)
    static void send_log(const Context& ctx, const std::string& msg)
    {
        packet::message::Log log_pkt{};
        log_pkt.msg = msg;
        packet::PacketHelper::write(log_pkt, ctx.server);
    }
};
}
