#pragma once
#include <cstdint>

#include "world_tile_map.h"
#include "../utils/binary_reader.h"

#pragma pack(push, 1)
struct World {
    uint16_t version;
    uint32_t unk;
    uint16_t name_len;
    std::string name;
    WorldTileMap tile_map;

    World() : version{ 0 }, unk{ 0 }, name_len{ 0 }, name{}, tile_map{} {}
    ~World() = default;

    void serialize(void* buffer) {
        BinaryReader br{ buffer };

        version = br.read_u16();
        unk = br.read_u32();
        name_len = br.read_u16();
        br.back(sizeof(uint16_t));
        name = br.read_string();

        tile_map.serialize(buffer, br.position());

        // TODO: Serialize world object.
        uint32_t object_count = br.read_u32();
        uint32_t object_drop_id = br.read_u32();

        if (object_count > 0) {
            for (uint32_t i = 0; i < object_count; i++) {
                uint16_t object_id = br.read_u16();
                auto object_pos = br.read<math::Vec2<int>>();
                uint8_t object_amount = br.read_u8();
                uint8_t object_unk = br.read_u8();
                uint32_t object_drop_id_offset = br.read_u32();
            }
        }

        uint32_t unk = br.read_u32();
        uint32_t unk2 = br.read_u32();
    }
};
#pragma pack(pop)
