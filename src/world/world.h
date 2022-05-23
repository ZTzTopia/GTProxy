#pragma once
#include <cstdint>

#include "world_object_map.h"
#include "world_tile_map.h"
#include "../utils/binary_reader.h"
#include "../utils/math.h"

#pragma pack(push, 1)
struct World {
    uint16_t version;
    uint32_t unk;
    uint16_t name_len;
    std::string name;
    WorldTileMap tile_map;
    WorldObjectMap object_map;

    World() : version(0), unk(0), name_len(0), name(), tile_map(), object_map() {}
    ~World()
    {
        tile_map.tiles.clear();
        object_map.objects.clear();
    }

    void serialize(void* buffer)
    {
        BinaryReader br{ buffer };

        version = br.read_u16();
        unk = br.read_u32();
        name_len = br.read_u16();
        br.back(sizeof(uint16_t));
        name = br.read_string();

        std::size_t position = br.position();
        tile_map.serialize(buffer, position, version);
        object_map.serialize(buffer, position);
    }
};
#pragma pack(pop)
