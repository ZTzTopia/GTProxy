#pragma once
#include <vector>

#include "object.h"
#include "../utils/math.h"
#include "../utils/binary_reader.h"

#pragma pack(push, 1)
struct Object {
    uint16_t item_id;
    math::Vec2<float> pos;
    uint8_t amount;
    uint8_t flags;
    uint32_t drop_id_offset;

    void serialize(void* buffer, std::size_t& position) {
        BinaryReader br{ buffer };
        br.skip(position);

        item_id = br.read_u16();
        pos = br.read<math::Vec2<float>>();
        amount = br.read_u8();
        flags = br.read_u8();
        drop_id_offset = br.read_u32();

        position = br.position();
    }
};
#pragma pack(pop)
