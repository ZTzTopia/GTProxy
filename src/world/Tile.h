#pragma once
#include <stdint.h>
#include "TileExtra.h"

#pragma pack(push, 1)
class Tile{
public:
    Tile() : foreground(0), background(0), parent_tile(0), flags(0) {
        tile_extra = new TileExtra();
    }
    ~Tile() {
        tile_extra = nullptr;
    }

    uint16_t foreground;
    uint16_t background;
    uint16_t parent_tile;
    uint16_t flags;
    
    TileExtra* tile_extra;
public:
    enum Flag {
        EXTRA_DATA = 1 << 0,
        LOCKED = 1 << 1,
        SEED = 1 << 4,
        FLIPPED = 1 << 5,
        OPEN = 1 << 6,
        PUBLIC = 1 << 7,
        SILENCED = 1 << 9,
        WATER = 1 << 10,
        GLUE = 1 << 11,
        FIRE = 1 << 12,
        RED = 1 << 13,
        GREEN = 1 << 14,
        BLUE = 1 << 15,
    };
};
#pragma pack(pop)