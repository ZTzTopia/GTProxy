#pragma once
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

#include "../player/player.hpp"
#include "../utils/singleton.hpp"
#include "tile_map.hpp"
#include "object_map.hpp"
#include "../utils/byte_stream.hpp"

namespace world {
class World : public utils::Singleton<World> {
public:
    World()
        : players_{}
        , local_net_id_{ -1 }
        , version_{ 0 }
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
        if (net_id != local_net_id_) {
            players_.erase(net_id);
        }
    }

    void clear()
    {
        players_.clear();
        local_net_id_ = -1;
        tile_map_ = WorldTileMap{};
        object_map_ = WorldObjectMap{};
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

    [[nodiscard]] WorldTileMap& get_tile_map()
    {
        return tile_map_;
    }

    [[nodiscard]] WorldObjectMap& get_object_map()
    {
        return object_map_;
    }

    [[nodiscard]] uint16_t get_version() const
    {
        return version_;
    }

    void serialize(const uint8_t* extended_data)
    {
        if (!extended_data) {
            return;
        }

        utils::ByteStream<> bs(reinterpret_cast<const std::byte*>(extended_data), std::numeric_limits<std::size_t>::max());

        bs.read(version_);

        tile_map_.serialize(bs, version_);
        object_map_.serialize(bs);
    }

private:
    std::unordered_map<int32_t, std::shared_ptr<player::Player>> players_;
    int32_t local_net_id_;
    uint16_t version_;
    WorldTileMap tile_map_;
    WorldObjectMap object_map_;
};
}
