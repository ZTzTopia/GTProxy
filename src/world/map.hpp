#pragma once
#include <string>

#include "object_map.hpp"
#include "tile_map.hpp"
#include "../utils/byte_stream.hpp"

class Map final {
public:
    Map()
        : version_{ 0 }
        , unk_{ 0 }
        , name_len_{ 0 }
        , world_owner_id_{ 0 }
    {

    }
    ~Map() = default;

    void serialize(utils::ByteStream<>& bs)
    {
        bs.read(version_);
        bs.read(unk_);
        bs.read(name_len_);
        bs.backtrack(sizeof(std::uint16_t));
        bs.read(name_);

        tile_map_.serialize(bs, version_);
        object_map_.serialize(bs);
    }

private:
    std::uint16_t version_;
    std::uint32_t unk_;
    std::uint16_t name_len_;
    std::string name_;
    WorldTileMap tile_map_;
    WorldObjectMap object_map_;

    std::uint32_t world_owner_id_;
};
