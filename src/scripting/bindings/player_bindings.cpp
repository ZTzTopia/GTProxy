#include "player_bindings.hpp"

namespace scripting::bindings {
void PlayerBindings::bind(sol::state& lua)
{
    lua.new_usertype<player::Player>("Player",
        sol::no_constructor,
        "net_id", sol::property(&player::Player::net_id),
        "user_id", sol::property(&player::Player::user_id),
        "name", sol::property(&player::Player::name),
        "country_code", sol::property(&player::Player::country_code),
        "position", sol::property([](const player::Player& p, const sol::this_state& this_state) {
            sol::state_view state{ this_state };
            sol::table t = state.create_table();
            t["x"] = p.position().x;
            t["y"] = p.position().y;
            return t;
        }),
        "collision", sol::property([](const player::Player& p, const sol::this_state& this_state) {
            sol::state_view state{ this_state };
            sol::table t = state.create_table();
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
}
