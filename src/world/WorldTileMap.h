#pragma once
#include <string>
#include <vector>
#include "Tile.h"

#pragma pack(push, 1)
class WorldTileMap {
public:
    CL_Vec2i size;
    char pad0[8];
public:
    std::vector<Tile> tiles;
};
#pragma pack(pop)