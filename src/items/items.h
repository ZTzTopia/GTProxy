#pragma once
#include <cstdint>
#include <vector>

#include "item_info.h"
#include "../utils/binary_reader.h"

namespace items {
    struct Items {
        uint16_t version;
        uint32_t count;
        std::vector<ItemInfo*> items;

        Items() : version(0), count(0), items() {}

        void serialize(void* buffer) {
            BinaryReader binary_reader{ buffer };
            version = binary_reader.read_u16();
            count = binary_reader.read_u32();

            items.reserve(count);

            std::size_t position = binary_reader.position();
            for (uint32_t i = 0; i < count; i++) {
                items.emplace_back(new ItemInfo{});
                items[i]->serialize(buffer, version, position);

                if (items[i]->id != i) {
                    spdlog::error("Item id mismatch: {} != {}", items[i]->id, i);
                    break;
                }
            }
        }

        [[nodiscard]] ItemInfo* get_item(uint32_t id) const {
            if (id >= count) {
                return nullptr;
            }

            return items[id];
        }
    };
}