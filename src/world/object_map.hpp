#pragma once
#include <cstdint>
#include <vector>

#include "../utils/byte_stream.hpp"
#include "object.hpp"

class WorldObjectMap final {
public:
    WorldObjectMap()
        : drop_id_{ 0 }
    {

    }

    void serialize(utils::ByteStream<>& bs)
    {
        std::uint32_t count{};
        bs.read(count);
        bs.read(drop_id_);

        objects_.resize(count);
        for (auto& object : objects_) {
            object.serialize(bs);
        }
    }

    [[nodiscard]] std::uint32_t get_drop_id() const { return drop_id_; }
    void set_drop_id(std::uint32_t id) { drop_id_ = id; }
    void increment_drop_id() { ++drop_id_; }
    [[nodiscard]] const std::vector<world::Object>& get_objects() const { return objects_; }
    [[nodiscard]] std::vector<world::Object>& get_objects() { return objects_; }

private:
    std::uint32_t drop_id_;
    std::vector<world::Object> objects_;
};
