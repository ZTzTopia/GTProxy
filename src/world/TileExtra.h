#pragma once
#include <cstdint>
#include "fmt/ranges.h"

#pragma pack(push, 1)
struct TileExtra {
    enum ExtraType : uint8_t {
        TYPE_NONE,
        TYPE_DOOR,
        TYPE_SIGN,
        TYPE_LOCK,
        TYPE_SEED,
        TYPE_UNK,
        TYPE_MAILBOX,
        TYPE_BULLETIN,
        TYPE_UNK2,
        TYPE_PROVIDER,
        TYPE_ACHIEVEMENT,
        TYPE_HEART_MONITOR,
        TYPE_DONATION_BOX,
        TYPE_MANNEQUIN,
        TYPE_TOYBOX,
        TYPE_GAME_GEN = 16,
        TYPE_XENONITE = 18,
        TYPE_DRESSUP,
        TYPE_CRYSTAL,
        TYPE_BURGLAR,
        TYPE_DISPLAY_BLOCK,
        TYPE_VENDING,
        TYPE_UNK3,
        TYPE_SOLAR,
        TYPE_UNK4,
        TYPE_DECO,
        TYPE_UNK5,
        TYPE_UNK6,
        TYPE_TAMAGOTCHI,
        TYPE_SEWING,
        TYPE_UNK7,
        TYPE_FLAG,
        TYPE_UNK8,
        TYPE_UNK9,
        TYPE_UNK10,
        TYPE_UNK11,
        TYPE_LOCK_BOT,
        TYPE_WEATHER_SPECIAL,
        TYPE_UNK12,
        TYPE_UNK13,
        TYPE_UNK14,
        TYPE_UNK15,
        TYPE_UNK16, // Guild?
        TYPE_UNK17,
        TYPE_UNK18,
        TYPE_UNK19,
        TYPE_UNK20,
        TYPE_UNK21,
        TYPE_UNK22,
        TYPE_STUFF_WEATHER = 49,
        TYPE_HOWLER = 52,
        TYPE_LUCKY_TOKEN = 56,
        TYPE_GEIGER_CHARGER,
        TYPE_ADVENTURE_BEGIN,
        TYPE_ADVENTURE_END,
        TYPE_SUCKER = 62,
        TYPE_ROBOT = 63,
        TYPE_GUILD_ITEM = 65,
        TYPE_GROW_SCAN,
        TYPE_SUCKER_2 = 71,
        TYPE_TEMP_PLATFORM = 73,
        TYPE_SAFE_VAULT,
        MAX,
        REAL_MAX = 83
    };

public:
    TileExtra() = default;
    ~TileExtra() = default;

    static std::string_view GetTypeAsString(uint8_t type) {
        std::array<std::string_view, MAX> types = {
            "TYPE_NONE", "TYPE_DOOR", "TYPE_SIGN", "TYPE_LOCK", "TYPE_SEED", "TYPE_UNK", "TYPE_MAILBOX",
            "TYPE_BULLETIN", "TYPE_UNK2", "TYPE_PROVIDER", "TYPE_ACHIEVEMENT", "TYPE_HEART_MONITOR",
            "TYPE_DONATION_BOX", "TYPE_MANNEQUIN", "TYPE_TOY_BOX", "TYPE_UNK3", "TYPE_XENONITE", "TYPE_DRESS_UP",
            "TYPE_CRYSTAL", "TYPE_BURGLAR", "TYPE_DISPLAY_BLOCK", "TYPE_VENDING_MACHINE", "TYPE_UNK4", "TYPE_SOLAR",
            "TYPE_UNK5", "TYPE_UNK6", "TYPE_TAMAGOTCHI", "TYPE_SEWING_MACHINE", "TYPE_UNK7", "TYPE_COUNTRY_FLAG",
            "TYPE_UNK8", "TYPE_UNK9", "TYPE_UNK10", "TYPE_UNK11", "TYPE_LOCK_BOT", "TYPE_WEATHER_BACKGROUND",
            "TYPE_UNK12", "TYPE_UNK13", "TYPE_UNK14", "TYPE_UNK15", "TYPE_UNK16", "TYPE_UNK17", "TYPE_UNK18",
            "TYPE_UNK19", "TYPE_UNK20", "TYPE_UNK21", "TYPE_UNK22", "TYPE_STUFF_WEATHER", "TYPE_HOWLER",
            "TYPE_LUCKY_TOKEN", "TYPE_GEIGER_CHARGER", "TYPE_ADVENTURE_BEGIN", "TYPE_ADVENTURE_END", "TYPE_SUCKER",
            "TYPE_ROBOT", "TYPE_GUILD_ITEM", "TYPE_GROW_SCAN", "TYPE_SUCKER_2", "TYPE_TEMP_PLATFORM",
            "TYPE_SAFE_VAULT"
        };

        if (type > MAX || type < TYPE_NONE)
            type = TYPE_NONE;

        return types[type];
    }

    [[nodiscard]] std::string GetRawData() const {
        switch (m_type) {
            case ExtraType::TYPE_DOOR:
                return fmt::format(" > label: {}\n > unk: {}", m_door.label, m_door.unk);
            case ExtraType::TYPE_SIGN:
                return fmt::format(" > label: {}\n > unk32: {}", m_sign.label, m_sign.unk);
            case ExtraType::TYPE_SEED:
                return fmt::format(" > growth_time: {}\n > fruit_count: {}", m_seed.growth_time, m_seed.fruit_count);
            case ExtraType::TYPE_PROVIDER:
                return fmt::format(" > unk: {}\n > unk2: {}", m_provider.unk, m_provider.unk2);
            case ExtraType::TYPE_HEART_MONITOR:
                return fmt::format(" > unk: {}\n > label: {}", m_heart_monitor.unk, m_heart_monitor.label);
            default:
                return " > None";
        }
    }

public:
    ExtraType m_type;

    struct {
        uint16_t label_len;
        std::string label;
        uint32_t unk;
    } m_door;

    struct {
        uint16_t label_len;
        std::string label;
        uint8_t unk;
    } m_sign;

    struct {
        uint32_t growth_time;
        uint8_t fruit_count;
    } m_seed;

    struct {
        uint32_t unk;
        uint32_t unk2;
    } m_provider;

    struct {
        uint32_t unk;
        uint16_t label_len;
        std::string label;
    } m_heart_monitor;
};
#pragma pack(pop)
