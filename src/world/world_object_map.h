#pragma once
#include <vector>

#include "object.h"
#include "../utils/binary_reader.h"

#pragma pack(push, 1)
struct WorldObjectMap {
    uint32_t count;
    uint32_t drop_id;
    std::vector<Object> objects;

    void serialize(void* buffer, std::size_t position) {
        BinaryReader br{ buffer };
        br.skip(position);

        count = br.read_u32();
        drop_id = br.read_u32();

        objects.clear();
        objects.reserve(count);

        position = br.position();
        for (uint32_t i = 0; i < count; i++) {
            Object object{};
            object.serialize(buffer, position);
            objects.push_back(object);
        }
    }
};
#pragma pack(pop)
