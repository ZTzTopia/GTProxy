#pragma once
#include <string>
#include <vector>
#include <charconv>
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
            return handle_player(ctx);
        }

        if (sub_cmd == "world") {
            return handle_world(ctx);
        }

        if (sub_cmd == "tile") {
            return handle_tile(ctx);
        }

        if (sub_cmd == "object") {
            return handle_object(ctx);
        }

        send_log(ctx, fmt::format("Unknown debug target: {}", sub_cmd));
        return Result::Failed;
    }

private:
    void send_log(const Context& ctx, const std::string& msg) const
    {
        packet::message::Log log_pkt{};
        log_pkt.msg = msg;
        packet::PacketHelper::write(log_pkt, ctx.server);
    }

    Result handle_player(const Context& ctx) const
    {
        auto& world = world::World::instance();
        const auto& players = world.get_players();

        if (ctx.args.size() > 1) {
            int32_t net_id{ 0 };
            const auto& arg{ ctx.args[1] };
            if (auto [ptr, ec] = std::from_chars(arg.data(), arg.data() + arg.size(), net_id); ec == std::errc{}) {
                if (auto player = world.get_player(net_id).lock()) {
                    send_log(ctx, fmt::format("Player Info (NetID: {}):", net_id));
                    send_log(ctx, fmt::format("Name: {} (User: {})", player->name(), player->user_id()));
                    send_log(ctx, fmt::format("Pos: {}, {}", player->position().x, player->position().y));
                    send_log(ctx, fmt::format("Country: {}", player->country_code()));
                    send_log(ctx, fmt::format("Mod: {}, SuperMod: {}", player->mod_state(), player->supermod_state()));
                    return Result::Success;
                } else {
                    send_log(ctx, fmt::format("Player with NetID {} not found.", net_id));
                    return Result::Failed;
                }
            } else {
                send_log(ctx, "Invalid NetID format.");
                return Result::Failed;
            }
        }

        send_log(ctx, fmt::format("Total Players: {}", players.size()));
        for (const auto& [net_id, player] : players) {
            send_log(ctx, fmt::format("[{}] {} ({}, {})", 
                net_id, player->name(), player->position().x, player->position().y));
        }
        return Result::Success;
    }

    Result handle_world(const Context& ctx) const
    {
        auto& world = world::World::instance();
        auto& tile_map = world.get_tile_map();
        auto& object_map = world.get_object_map();

        send_log(ctx, "World Info:");
        send_log(ctx, fmt::format("Version: {}", world.get_version()));
        send_log(ctx, fmt::format("Size: {}x{}", tile_map.get_size().x, tile_map.get_size().y));
        send_log(ctx, fmt::format("Tiles: {}", tile_map.get_tiles().size()));
        send_log(ctx, fmt::format("Objects: {}", object_map.get_objects().size()));
        send_log(ctx, fmt::format("Player Count: {}", world.get_players().size()));
        
        return Result::Success;
    }

    Result handle_tile(const Context& ctx) const
    {
        auto& world = world::World::instance();
        auto& tile_map = world.get_tile_map();
        
        int x = 0, y = 0;

        if (ctx.args.size() >= 3) {
            const auto& arg_x{ ctx.args[1] };
            const auto& arg_y{ ctx.args[2] };
            auto [ptr_x, ec_x] = std::from_chars(arg_x.data(), arg_x.data() + arg_x.size(), x);
            auto [ptr_y, ec_y] = std::from_chars(arg_y.data(), arg_y.data() + arg_y.size(), y);

            if (ec_x != std::errc{} || ec_y != std::errc{}) {
                send_log(ctx, "Invalid coordinates.");
                return Result::Failed;
            }
        } else {
            if (auto local_player = world.get_local_player().lock()) {
                x = local_player->position().x / 32;
                y = local_player->position().y / 32;
            } else {
                send_log(ctx, "Local player not found.");
                return Result::Failed;
            }
        }

        if (x < 0 || x >= tile_map.get_size().x || y < 0 || y >= tile_map.get_size().y) {
            send_log(ctx, fmt::format("Coordinates {},{} out of bounds.", x, y));
            return Result::Failed;
        }

        const auto& tiles = tile_map.get_tiles();
        size_t index = x + y * tile_map.get_size().x;

        if (index < tiles.size()) {
            const auto& tile = tiles[index];
            send_log(ctx, fmt::format("Tile at {},{}:", x, y));
            send_log(ctx, fmt::format("FG: {} BG: {}", tile.foreground, tile.background));
            send_log(ctx, fmt::format("Flags: {}", static_cast<uint16_t>(tile.flag)));
            if (tile.parent_tile != 0) {
                 send_log(ctx, fmt::format("Parent: {}", tile.parent_tile));
            }
        } else {
             send_log(ctx, "Tile index out of range (internal error).");
        }

        return Result::Success;
    }

    Result handle_object(const Context& ctx) const
    {
        auto& world = world::World::instance();
        auto& object_map = world.get_object_map();
        const auto& objects = object_map.get_objects();

        if (ctx.args.size() > 1 && ctx.args[1] == "list") {
             send_log(ctx, fmt::format("Total Objects: {}", objects.size()));
             int count = 0;
             for (const auto& obj : objects) {
                 if (count++ > 10) {
                     send_log(ctx, "... and more");
                     break;
                 }
                 send_log(ctx, fmt::format("ID: {} Type: {} Pos: {},{}", 
                    obj.object_id, obj.item_id, obj.pos.x, obj.pos.y));
             }
             return Result::Success;
        }

        if (auto local_player = world.get_local_player().lock()) {
            float min_dist = std::numeric_limits<float>::max();
            const world::Object* nearest = nullptr;
            glm::vec2 p_pos = { (float)local_player->position().x, (float)local_player->position().y };

            for (const auto& obj : objects) {
                float dist = glm::distance(p_pos, obj.pos);
                if (dist < min_dist) {
                    min_dist = dist;
                    nearest = &obj;
                }
            }

            if (nearest) {
                send_log(ctx, "Nearest Object:");
                send_log(ctx, fmt::format("ID: {}", nearest->object_id));
                send_log(ctx, fmt::format("Item ID: {}", nearest->item_id));
                send_log(ctx, fmt::format("Pos: {}, {}", nearest->pos.x, nearest->pos.y));
                send_log(ctx, fmt::format("Amount: {}", nearest->amount));
                send_log(ctx, fmt::format("Flags: {}", nearest->flags));
            } else {
                send_log(ctx, "No objects found.");
            }
        } else {
            send_log(ctx, "Local player not found.");
        }

        return Result::Success;
    }
};
}
