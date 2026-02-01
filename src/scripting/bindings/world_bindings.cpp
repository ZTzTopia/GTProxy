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

            uint8_t index{ 1 };
            for (const auto& player: world.get_players() | std::views::values) {
                players[index++] = player;
            }

            return players;
        }
    );

    lua["world"] = std::ref(world::World::instance());
}
}
