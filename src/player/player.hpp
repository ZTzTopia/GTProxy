#pragma once
#include <cstdint>
#include <string>
#include <glm/glm.hpp>

#include "../packet/game/world.hpp"

namespace player {
class Player {
public:
    Player()
        : net_id_{ -1 }
        , user_id_{ 0 }
        , position_{ 0, 0 }
        , collision_{ 0, 0, 0, 0 }
        , invisible_{ 0 }
        , mod_state_{ 0 }
        , supermod_state_{ 0 }
        , is_local_{ false }
    {

    }

    static std::shared_ptr<Player> from_on_spawn(const packet::game::OnSpawn& pkt)
    {
        auto player{ std::make_shared<Player>() };
        player->net_id_ = pkt.net_id;
        player->user_id_ = pkt.user_id;
        player->name_ = pkt.name;
        player->country_code_ = pkt.country_code;
        player->position_ = pkt.position;
        player->collision_ = pkt.collision;
        player->invisible_ = pkt.invisible;
        player->mod_state_ = pkt.mod_state;
        player->supermod_state_ = pkt.supermod_state;
        player->is_local_ = pkt.type == "local";
        return player;
    }

public:
    [[nodiscard]] int32_t net_id() const noexcept { return net_id_; }
    [[nodiscard]] bool is_local() const noexcept { return is_local_; }
    [[nodiscard]] int32_t user_id() const noexcept { return user_id_; }
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] const std::string& country_code() const noexcept { return country_code_; }
    [[nodiscard]] const glm::i32vec2& position() const noexcept { return position_; }
    [[nodiscard]] const glm::i32vec4& collision() const noexcept { return collision_; }
    [[nodiscard]] int32_t invisible() const noexcept { return invisible_; }
    [[nodiscard]] int32_t mod_state() const noexcept { return mod_state_; }
    [[nodiscard]] int32_t supermod_state() const noexcept { return supermod_state_; }

private:
    int32_t net_id_;
    int32_t user_id_;
    std::string name_;
    std::string country_code_;
    glm::i32vec2 position_;
    glm::i32vec4 collision_;
    int32_t invisible_;
    int32_t mod_state_;
    int32_t supermod_state_;
    bool is_local_;
};
}
