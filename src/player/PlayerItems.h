#pragma once
#include <cstdint>
#include <map>

#pragma pack(push, 1)
struct PlayerItems {
    uint32_t max_size;
    uint32_t size;
    std::map<uint16_t, std::pair<uint8_t, uint8_t>> items; // item id, (count, unknown TODO!)
};
#pragma pack(pop)
