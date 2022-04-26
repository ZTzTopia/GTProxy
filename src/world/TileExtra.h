#pragma once
#include <stdint.h>
#include "fmt/ranges.h"

#pragma pack(push, 1)
class TileExtra {
public:
    enum ExtraType : uint8_t {
        TYPE_NONE,
        TYPE_DOOR,
        TYPE_SIGN,
        TYPE_LOCK
    };
    static const char* GetTypeAsString(uint8_t type) {
        const char* types[] { 
            "TYPE_NONE", "TYPE_DOOR", "TYPE_SIGN", "TYPE_LOCK"
        };
        if (type > TYPE_LOCK || type < TYPE_NONE)
            type = TYPE_NONE;
        return types[type];
    }
    std::string GetRawData() const {
        switch(this->type) {
            case ExtraType::TYPE_DOOR: {
                return fmt::format(" > label: {}\n > unk_1: {}", this->label, this->unk_1);
            }
            case ExtraType::TYPE_SIGN: {
                return fmt::format(" > label: {}\n > unk_2: {}", this->label, this->unk_2);
            }
            case ExtraType::TYPE_LOCK: {
                return fmt::format(" > flags_1: {}\n > user_id: {}\n > access_list: {}", this->flags_1, this->user_id, this->access_list);
            }
        }
        return "None";
    }
public:
    TileExtra() = default;
    ~TileExtra() = default;

    ExtraType type{};
    uint8_t flags_1;
    union {
        uint32_t user_id{};
        uint32_t monitor_owner;
    };
    std::string label{};
    uint8_t unk_1{};
    uint32_t unk_2;
    std::vector<uint32_t> access_list;
    char unk7a[8];
};
#pragma pack(pop)