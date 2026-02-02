#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

#include "../utils/byte_stream.hpp"
#include "tile.hpp"

namespace world {
struct Object {
    std::uint16_t item_id;
    glm::vec2 pos;
    std::uint8_t amount;
    std::uint8_t flags;
    std::uint32_t object_id;

    Object()
        : item_id{ 0 }
        , pos{ 0.0f, 0.0f }
        , amount{ 0 }
        , flags{ 0 }
        , object_id{ 0 }
    {

    }

    [[nodiscard]] bool operator==(const Object& other) const
    {
        return item_id == other.item_id && pos == other.pos && amount == other.amount &&
               flags == other.flags && object_id == other.object_id;
    }

    void serialize(utils::ByteStream<>& bs)
    {
        bs.read(item_id);
        bs.read(pos.x);
        bs.read(pos.y);
        bs.read(amount);
        bs.read(flags);
        bs.read(object_id);
    }
};
}
