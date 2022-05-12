#pragma once
#include <vector>

#include "tile.h"
#include "../utils/math.h"

#pragma pack(push, 1)
struct WorldTileMap {
    math::Vec2<int> size;
    uint32_t tile_count;
    std::vector<Tile> tiles;

    void serialize(void* buffer, std::size_t position, uint16_t version) {
        BinaryReader br{ buffer };
        br.skip(position);

        size = br.read<math::Vec2<int>>();
        tile_count = br.read<uint32_t>();

        tiles.clear();
        tiles.reserve(tile_count);

        position = br.position();
        for (uint32_t i = 0; i < tile_count; i++) {
            Tile tile{};
            tile.serialize(buffer, position, version);
            tiles.push_back(tile);
        }
    }
};
#pragma pack(pop)
