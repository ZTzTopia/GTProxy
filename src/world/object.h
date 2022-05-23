#pragma once
#include <vector>

#include "../utils/math.h"
#include "../utils/binary_reader.h"

#pragma pack(push, 1)
struct Object {
    uint16_t item_id;
    utils::math::Vec2<float> pos;
    uint8_t amount;
    uint8_t flags;
    uint32_t drop_id_offset;

    Object() : item_id(0), pos(), amount(0), flags(0), drop_id_offset(0) {}
    ~Object() = default;

    bool operator==(const Object& other) const
    {
        return item_id == other.item_id && pos == other.pos && amount == other.amount
            && flags == other.flags && drop_id_offset == other.drop_id_offset;
    }

    void serialize(void* buffer, std::size_t& position)
    {
        BinaryReader br{ buffer };
        br.skip(position);

        item_id = br.read_u16();
        pos = br.read<utils::math::Vec2<float>>();
        amount = br.read_u8();
        flags = br.read_u8();
        drop_id_offset = br.read_u32();

        position = br.position();
    }
};
#pragma pack(pop)
