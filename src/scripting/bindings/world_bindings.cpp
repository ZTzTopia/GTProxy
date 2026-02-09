#include "world_bindings.hpp"

namespace scripting::bindings {
void WorldBindings::bind(sol::state& lua)
{
    lua.new_usertype<world::World>("World",
        sol::no_constructor,
        "get_local_player", [](const world::World& world) -> std::shared_ptr<player::Player> {
            return world.get_local_player().lock();
        },
        "get_local_net_id", &world::World::get_local_net_id,
        "get_player", [](const world::World& world, const int32_t net_id) -> std::shared_ptr<player::Player> {
            return world.get_player(net_id).lock();
        },
        "get_players", [](const world::World& world, const sol::this_state& this_state) {
            sol::state_view state{ this_state };
            sol::table players{ state.create_table() };

            for (const auto& [net_id, player] : world.get_players()) {
                players[net_id] = player;
            }

            return players;
        },
        "get_tile_map", [](world::World& world) -> WorldTileMap& {
            return world.get_tile_map();
        },
        "get_object_map", [](world::World& world) -> WorldObjectMap& {
            return world.get_object_map();
        },
        "get_version", &world::World::get_version
    );

    lua["world"] = std::ref(world::World::instance());
}
}
