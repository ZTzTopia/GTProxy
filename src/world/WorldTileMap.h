#pragma once
#include <vector>

#include "Tile.h"

class WorldTileMap {
public:
    CL_Vec2i size;
    uint32_t tile_count;
    std::vector<Tile> tiles;
};