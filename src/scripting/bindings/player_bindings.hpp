#pragma once
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "../binding_module.hpp"
#include "../../player/player.hpp"

namespace scripting::bindings {
class PlayerBindings final : public IBindingModule {
public:
    [[nodiscard]] std::string_view name() const override { return "player"; }

    void bind(sol::state& lua) override
    {
        lua.new_usertype<player::Player>("Player",
            sol::no_constructor,
            "net_id", sol::property(&player::Player::net_id),
            "user_id", sol::property(&player::Player::user_id),
            "name", sol::property(&player::Player::name),
            "country_code", sol::property(&player::Player::country_code),
            "position", sol::property([](const player::Player& p, sol::this_state s) {
                sol::state_view lua{ s };
                sol::table t = lua.create_table();
                t["x"] = p.position().x;
                t["y"] = p.position().y;
                return t;
            }),
            "collision", sol::property([](const player::Player& p, sol::this_state s) {
                sol::state_view lua{ s };
                sol::table t = lua.create_table();
                t["x"] = p.collision().x;
                t["y"] = p.collision().y;
                t["z"] = p.collision().z;
                t["w"] = p.collision().w;
                return t;
            }),
            "invisible", sol::property(&player::Player::invisible),
            "mod_state", sol::property(&player::Player::mod_state),
            "supermod_state", sol::property(&player::Player::supermod_state),
            "is_local", sol::property(&player::Player::is_local)
        );
    }
};
}
