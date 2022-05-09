#pragma once
#include <cstdint>
#include <array>
#include <fmt/format.h>

#pragma pack(push, 1)
struct TileExtra {
    enum eType : uint8_t {
        NONE,
        DOOR,
        SIGN,
        LOCK,
        SEED,
        UNK,
        MAILBOX,
        BULLETIN,
        UNK2,
        PROVIDER,
        ACHIEVEMENT,
        HEART_MONITOR,
        DONATION_BOX,
        MANNEQUIN,
        TOYBOX,
        GAME_GEN = 16,
        XENONITE = 18,
        DRESSUP,
        CRYSTAL,
        BURGLAR,
        DISPLAY_BLOCK,
        VENDING,
        UNK3,
        SOLAR,
        UNK4,
        DECO,
        UNK5,
        UNK6,
        TAMAGOTCHI,
        SEWING,
        UNK7,
        FLAG,
        UNK8,
        UNK9,
        UNK10,
        UNK11,
        LOCK_BOT,
        WEATHER_SPECIAL,
        UNK12,
        UNK13,
        UNK14,
        UNK15,
        UNK16, // Guild?
        UNK17,
        UNK18,
        UNK19,
        UNK20,
        UNK21,
        UNK22,
        STUFF_WEATHER = 49,
        HOWLER = 52,
        LUCKY_TOKEN = 56,
        GEIGER_CHARGER,
        ADVENTURE_BEGIN,
        ADVENTURE_END,
        SUCKER = 62,
        ROBOT = 63,
        GUILD_ITEM = 65,
        GROW_SCAN,
        SUCKER_2 = 71,
        TEMP_PLATFORM = 73,
        SAFE_VAULT,
        MAX
    };

    eType type;

    struct {
        uint16_t label_len;
        std::string label;
        uint32_t unk;
    } door;

    struct {
        uint16_t label_len;
        std::string label;
        uint8_t unk;
    } sign;

    struct {
        uint32_t growth_time;
        uint8_t fruit_count;
    } seed;

    struct {
        uint32_t unk;
        uint32_t unk2;
    } provider;

    struct {
        uint32_t unk;
        uint16_t label_len;
        std::string label;
    } heart_monitor;

    TileExtra() : type{ NONE }, door{}, sign{}, seed{}, provider{}, heart_monitor{} {}
    ~TileExtra() = default;

    void serialize(void* buffer, std::size_t& position) {
        BinaryReader br{ buffer };
        br.skip(position);

        type = static_cast<TileExtra::eType>(br.read_u8());
        switch (type) {
            case TileExtra::DOOR:
                door.label = br.read_string();
                door.unk = br.read_u8();
                break;
            case TileExtra::SIGN:
                sign.label = br.read_string();
                sign.unk = br.read_u32();
                break;
            case TileExtra::SEED:
                seed.growth_time = br.read_u32();
                seed.fruit_count = br.read_u8();
                break;
            case TileExtra::PROVIDER: {
                provider.unk = br.read_u32();

                /*World* world = player->get_world();
                if (foreground != 5318 && (foreground != 10656 || world->version < 17) )
                    break;*/

                provider.unk2 = br.read_u32();
                break;
            }
            case TileExtra::HEART_MONITOR:
                heart_monitor.unk = br.read_u32();
                heart_monitor.label = br.read_string();
                break;
            default:
                break;
        }

        position = br.position();
    }

    static std::string_view type_to_string(uint8_t index) {
        std::array<std::string, eType::MAX> types{
            "NONE", "DOOR", "SIGN", "SEED", "PROVIDER", "HEART_MONITOR", "DONATION_BOX",
            "MANNEQUIN", "TOYBOX", "GAME_GEN", "XENONITE", "DRESSUP", "CRYSTAL", "BURGLAR",
            "DISPLAY_BLOCK", "VENDING", "UNK3", "SOLAR", "UNK4", "DECO", "UNK5", "UNK6",
            "TAMAGOTCHI", "SEWING", "UNK7", "FLAG", "UNK8", "UNK9", "UNK10", "UNK11", "LOCK_BOT",
            "WEATHER_SPECIAL", "UNK12", "UNK13", "UNK14", "UNK15", "UNK16", "UNK17", "UNK18",
            "UNK19", "UNK20", "UNK21", "UNK22", "STUFF_WEATHER", "HOWLER", "LUCKY_TOKEN",
            "GEIGER_CHARGER", "ADVENTURE_BEGIN", "ADVENTURE_END", "SUCKER", "ROBOT", "GUILD_ITEM",
            "GROW_SCAN", "SUCKER_2", "TEMP_PLATFORM", "SAFE_VAULT", "MAX"
        };

        if (index > MAX || index < NONE)
            index = NONE;

        return types[index];
    }

    [[nodiscard]] std::string get_raw_data() const {
        switch (type) {
            case eType::DOOR:
                return fmt::format(" > label: {}\n > unk: {}", door.label, door.unk);
            case eType::SIGN:
                return fmt::format(" > label: {}\n > unk32: {}", sign.label, sign.unk);
            case eType::SEED:
                return fmt::format(" > growth_time: {}\n > fruit_count: {}", seed.growth_time, seed.fruit_count);
            case eType::PROVIDER:
                return fmt::format(" > unk: {}\n > unk2: {}", provider.unk, provider.unk2);
            case eType::HEART_MONITOR:
                return fmt::format(" > unk: {}\n > label: {}", heart_monitor.unk, heart_monitor.label);
            default:
                return " > None";
        }
    }
};
#pragma pack(pop)
