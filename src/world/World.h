#pragma once
#include <cstdint>
#include <unordered_map>

#include "WorldTileMap.h"
#include "../utils/binary_reader.h"

#pragma pack(push, 1)
struct World {
    uint16_t version;
    uint32_t unk;
    uint16_t name_len;
    std::string name;
    WorldTileMap* tile_map;

    World() : version(0), unk(0), name_len(0), name(), tile_map() { tile_map = new WorldTileMap{}; }
    ~World() { delete tile_map; }

    void serialize(void* buffer) {
        BinaryReader br{ buffer };

        version = br.read_u16();
        unk = br.read_u32();
        name_len = br.read_u16();
        br.back(sizeof(uint16_t));
        name = br.read_string();
    }
};
#pragma pack(pop)
