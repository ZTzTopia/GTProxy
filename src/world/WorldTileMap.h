#pragma once
#include <vector>

#include "Tile.h"
#include "../utils/math.h"

#pragma pack(push, 1)
struct WorldTileMap {
    math::Vec2<int> size;
    uint32_t tile_count;
    std::vector<Tile> tiles;
};
#pragma pack(pop)
