#pragma once
#include <cstdint>

#include "TileExtra.h"

class Tile {
public:
    enum TileFlag : uint16_t {
        NONE = 0,
        EXTRA = 1 << 0,
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

public:
    Tile() : foreground(0), background(0), parent_tile(0), flags(NONE) {
        tile_extra = new TileExtra{};
    }
    ~Tile() {
        delete tile_extra;
    }

public:
    uint16_t foreground;
    uint16_t background;
    uint16_t parent_tile;
    TileFlag flags;
    TileExtra* tile_extra;
};
