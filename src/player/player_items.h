#pragma once
#include <cstdint>
#include <unordered_map>

#include "../utils/binary_reader.h"

#pragma pack(push, 1)
struct PlayerItems {
    uint8_t version;
    uint32_t max_size;
    uint32_t size;
    std::unordered_map<uint16_t, std::pair<uint8_t, uint8_t>> items; // item id, (count, unused)

    PlayerItems() : version(0), max_size(0), size(0) {}

    void serialize(void* buffer) {
        BinaryReader binary_reader{ buffer };
        version = binary_reader.read_u8();
        max_size = binary_reader.read_u32();

        if (version > 0)
            size = binary_reader.read_u16();
        else
            size = binary_reader.read_u8();

        for (uint32_t i = 0; i < size; i++) {
            uint16_t id = binary_reader.read_u16();
            uint8_t count = binary_reader.read_u8();
            uint8_t unused = binary_reader.read_u8();
            items.insert_or_assign(id, std::make_pair(count, unused));
        }
    }
};
#pragma pack(pop)
