#pragma once
#include <cstdint>
#include <unordered_map>

#include "WorldTileMap.h"
#include "../utils/binary_reader.h"

class World {
public:
    World() : version(0), unk(0), name_len(0), name(), tile_map() { tile_map = new WorldTileMap{}; }
    ~World() { delete tile_map; }

    void serialize(uint8_t* data, uint32_t data_size) {
        BinaryReader br{ data, data_size };

        version = br.read_ushort();
        if (version < 20) {
            return;
        }

        unk = br.read_uint();
        name_len = br.read_ushort();
        br.back(sizeof(uint16_t));
        name = br.read_string();
    }
    
public:
    uint16_t version;
    uint32_t unk;
    uint16_t name_len;
    std::string name;
    WorldTileMap* tile_map;
};
