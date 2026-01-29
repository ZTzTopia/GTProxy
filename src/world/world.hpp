#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>

#include "../player/player.hpp"
#include "../utils/singleton.hpp"

namespace world {
class World : public utils::Singleton<World> {
public:
    World()
        : local_net_id_{ -1 }
    {

    }

    void add_player(const std::shared_ptr<player::Player>& player)
    {
        if (player->is_local()) {
            local_net_id_ = player->net_id();
        }

        players_[player->net_id()] = player;
    }

    void remove_player(const int32_t net_id)
    {
        // Don't remove local player (they never receive OnRemove for themselves)
        if (net_id != local_net_id_) {
            players_.erase(net_id);
        }
    }

    void clear()
    {
        players_.clear();
        local_net_id_ = -1;
    }

    [[nodiscard]] std::weak_ptr<player::Player> get_player(const int32_t net_id) const
    {
        if (const auto it{ players_.find(net_id) }; it != players_.end()) {
            return it->second;
        }

        return {};
    }

    [[nodiscard]] std::weak_ptr<player::Player> get_local_player() const
    {
        if (local_net_id_ < 0) {
            return {};
        }

        return get_player(local_net_id_);
    }

    [[nodiscard]] const std::unordered_map<int32_t, std::shared_ptr<player::Player>>& get_players() const
    {
        return players_;
    }

    [[nodiscard]] int32_t get_local_net_id() const
    {
        return local_net_id_;
    }

private:
    std::unordered_map<int32_t, std::shared_ptr<player::Player>> players_;
    int32_t local_net_id_;
};
}
