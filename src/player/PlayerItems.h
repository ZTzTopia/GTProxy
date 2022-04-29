#pragma once
#include <cstdint>
#include <unordered_map>

#pragma pack(push, 1)
struct PlayerItems {
    uint8_t version;
    uint32_t max_size;
    uint32_t size;
    std::unordered_map<uint16_t, std::pair<uint8_t, uint8_t>> items; // item id, (count, unused)
};
#pragma pack(pop)
