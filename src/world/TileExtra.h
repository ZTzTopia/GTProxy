#pragma once
#include <cstdint>
#include "fmt/ranges.h"

#pragma pack(push, 1)
class TileExtra {
public:
    enum ExtraType : uint8_t {
        TYPE_NONE,
        TYPE_DOOR,
        TYPE_SIGN,
        TYPE_LOCK,
        TYPE_TREE,
        TYPE_UNK,
        TYPE_MAILBOX,
        TYPE_BULLETIN,
        TYPE_UNK2,
        TYPE_PROVIDER,
        TYPE_ACHIEVEMENT_BLOCK,
        TYPE_HEART_MONITOR,
        TYPE_DONATION_BOX,
        TYPE_MANNEQUIN,
        TYPE_TOY_BOX,
        TYPE_UNK3 = 16,
        MAX,
        REAL_MAX = 83
    };

public:
    TileExtra() = default;
    ~TileExtra() {
        access.clear();
    }

    static std::string_view GetTypeAsString(uint8_t type) {
        std::array<std::string_view, MAX> types = {
            "TYPE_NONE", "TYPE_DOOR", "TYPE_SIGN", "TYPE_LOCK", "TYPE_TREE", "TYPE_UNK", "TYPE_MAILBOX",
            "TYPE_BULLETIN", "TYPE_UNK2", "TYPE_PROVIDER", "TYPE_ACHIEVEMENT_BLOCK", "TYPE_HEART_MONITOR",
            "TYPE_DONATION_BOX", "TYPE_MANNEQUIN", "TYPE_TOY_BOX", "TYPE_UNK3"
        };

        if (type > MAX || type < TYPE_NONE)
            type = TYPE_NONE;

        return types[type];
    }

    [[nodiscard]] std::string GetRawData() const {
        switch (this->type) {
            case ExtraType::TYPE_DOOR: {
                return fmt::format(" > label: {}\n > unk: {}", this->label, this->unk_2);
            }
            case ExtraType::TYPE_SIGN: {
                return fmt::format(" > label: {}\n > unk32: {}", this->label, this->unk32_2);
            }
            case ExtraType::TYPE_LOCK: {
                // return fmt::format(" > flags_1: {}\n > user_id: {}\n > access_list: {}", this->flags_1, this->user_id, this->access_list);
            }
            case ExtraType::TYPE_TREE: {
                return fmt::format(" > growth_time: {}\n > fruit_count: {}", this->growth_time, this->fruit_count);
            }
        }
        return "None";
    }

public:
    ExtraType type;
    union {
        uint16_t label_len{};
        uint32_t growth_time;
    };
    union {
        uint8_t unk{};
        uint8_t fruit_count;
        uint32_t user_id;
        std::string label;
    };
    union {
        uint8_t unk_2{};
        uint32_t unk32_2;
        uint32_t access_count;
    };
    std::vector<uint32_t> access;
};
#pragma pack(pop)