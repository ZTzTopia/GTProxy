#pragma once
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "../binding_module.hpp"
#include "../../world/world.hpp"

namespace scripting::bindings {
class WorldBindings final : public IBindingModule {
public:
    [[nodiscard]] std::string_view name() const override { return "world"; }

    void bind(sol::state& lua) override
    {
        auto world_type{ lua.new_usertype<world::World>("World",
            sol::no_constructor,
            "get_local_player", [](const world::World& w) -> std::shared_ptr<player::Player> {
                return w.get_local_player().lock();
            },
            "get_local_net_id", &world::World::get_local_net_id,
            "get_player", [](const world::World& w, const int32_t net_id) -> std::shared_ptr<player::Player> {
                return w.get_player(net_id).lock();
            },
            "get_players", [](world::World& w, sol::this_state s) {
                sol::state_view lua{ s };
                sol::table players = lua.create_table();
                int index = 1;
                for (const auto& [net_id, player] : w.get_players()) {
                    players[index++] = player;
                }
                return players;
            }
        ) };

        lua["world"] = std::ref(world::World::instance());
    }
};
}
