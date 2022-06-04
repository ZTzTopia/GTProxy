#pragma once
#include <cstdint>
#include <array>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#pragma pack(push, 1)
struct TileExtra {
    enum eType : uint8_t {
        NONE,
        DOOR,
        SIGN,
        LOCK,
        SEED,
        MAILBOX = 6, // TODO!
        BULLETIN, // TODO!
        DICE,
        PROVIDER,
        ACHIEVEMENT,
        HEART_MONITOR,
        DONATION_BOX, // TODO!
        TOYBOX, // TODO!
        MANNEQUIN, // TODO!
        BUNNY_EGG,
        GAME_GEN,
        XENONITE = 18,
        DRESSUP, // TODO!
        CRYSTAL,
        BURGLAR,
        SPOTLIGHT,
        DISPLAY_BLOCK,
        VENDING,
        SOLAR,
        DECO = 28,
        SEWING_MACHINE = 32,
        COUNTRY_FLAG,
        BATTLE_CAGE = 36,
        WEATHER_SPECIAL = 40,
        VIP_ENTRANCE = 44,
        WEATHER_SPECIAL2 = 49, // TODO!
        HOWLER = 52,
        GEIGER_CHARGER = 57,
        SUCKER = 62, // TODO!
        ROBOT, // TODO!
        GROWSCAN9000 = 66, // TODO!
        SUCKER2 = 71, // TODO!
        SAFE_VAULT = 74, // TODO!
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
        uint32_t unk;
        uint32_t owner_id;
        uint32_t access_size;
        std::vector<uint32_t> accesses;
    } lock;

    struct {
        uint32_t growth_time;
        uint8_t fruit_count;
    } seed;

    struct {
        uint8_t number;
    } dice;

    struct {
        uint32_t unk;
        uint32_t unk2;
    } provider;

    struct {
        uint32_t unk;
        uint8_t unk2;
    } achievement;

    struct {
        uint32_t unk;
        uint16_t label_len;
        std::string label;
    } heart_monitor;

    struct {
        uint32_t unk;
    } bunny_egg;

    struct {
        uint8_t unk;
    } game_gen;

    struct {
        uint8_t unk;
        uint32_t unk2;
    } xenonite;

    struct {
        uint16_t len;
        std::string unk;
    } crystal;

    struct {
        uint16_t len;
        std::string unk;
        uint32_t unk2;
        uint8_t unk3;
    } burglar;

    struct {
        uint32_t item_id;
    } display_block;

    struct {
        uint32_t item_id;
        uint32_t price;
    } vending;

    struct {
        uint8_t unk;
        uint32_t unk2;
        std::vector<uint32_t> unk3;
    } solar;

    struct {
        uint8_t unk;
        uint32_t unk2;
        uint8_t unk3;
    } deco;

    struct {
        uint32_t unk;
    } sewing_machine;

    struct {
        uint16_t len;
        std::string flag;
    } country_flag;

    struct {
        uint16_t len;
        std::string label;
        uint32_t pet1;
        uint32_t pet2;
        uint32_t pet3;
    } battle_cage;

    struct {
        union {
            uint32_t color;
            uint32_t item_id;
        };
    } weather_special;

    struct {
        uint8_t unk;
        uint32_t owner_id;
        uint32_t access_size;
        std::vector<uint32_t> accesses;
    } vip_entrance;

    struct {
        uint32_t unk;
    } geiger_charger;

    TileExtra()
        : type(NONE), door(), sign(), lock(), seed(), dice(), provider(), achievement(), heart_monitor(), bunny_egg()
        , game_gen(), xenonite(), crystal(), burglar(), display_block(), vending(), solar(), deco(), sewing_machine()
        , country_flag(), battle_cage(), weather_special(), vip_entrance(), geiger_charger() {}
    ~TileExtra() = default;

    void serialize(void* buffer, std::size_t& position, uint16_t version, const std::pair<uint16_t, uint16_t>& fg_bg)
    {
        BinaryReader br{ buffer };
        br.skip(position);

        type = static_cast<TileExtra::eType>(br.read_u8());

        spdlog::debug("TileExtra::serialize: type: {}", type);

        switch (type) {
            case TileExtra::DOOR:
                door.label = br.read_string();
                door.unk = br.read_u8();
                break;
            case TileExtra::SIGN:
                sign.label = br.read_string();
                sign.unk = br.read_u32();
                break;
            case TileExtra::LOCK: {
                lock.unk = br.read_u8();
                lock.owner_id = br.read_u32();
                lock.access_size = br.read_u32();

                lock.accesses.reserve(lock.access_size);

                for (uint32_t i = 0; i < lock.access_size; i++) {
                    lock.accesses.push_back(br.read_u32());
                }

                br.skip(8);
                if (fg_bg.first == 5814) // Guild Lock
                    br.skip(16);

                break;
            }
            case TileExtra::SEED:
                seed.growth_time = br.read_u32();
                seed.fruit_count = br.read_u8();
                break;
            case TileExtra::DICE:
                dice.number = br.read_u8();
                break;
            case TileExtra::PROVIDER: {
                provider.unk = br.read_u32();

                if (fg_bg.first != 5318 && (fg_bg.first != 10656 || version < 17))
                    break;

                provider.unk2 = br.read_u32();
                break;
            }
            case TileExtra::ACHIEVEMENT:
                achievement.unk = br.read_u32();
                achievement.unk2 = br.read_u8();
                break;
            case TileExtra::HEART_MONITOR:
                heart_monitor.unk = br.read_u32();
                heart_monitor.label = br.read_string();
                break;
            case TileExtra::BUNNY_EGG:
                bunny_egg.unk = br.read_u32();
                break;
            case TileExtra::GAME_GEN:
                game_gen.unk = br.read_u8();
                break;
            case TileExtra::XENONITE:
                xenonite.unk = br.read_u8();
                xenonite.unk2 = br.read_u32();
                break;
            case TileExtra::CRYSTAL:
                crystal.unk = br.read_string();
                break;
            case TileExtra::BURGLAR:
                burglar.unk = br.read_string();
                burglar.unk2 = br.read_u32();
                burglar.unk3 = br.read_u8();
                break;
            case TileExtra::DISPLAY_BLOCK:
                display_block.item_id = br.read_u32();
                break;
            case TileExtra::VENDING:
                vending.item_id = br.read_u32();
                vending.price = br.read_u32();
                break;
            case TileExtra::SOLAR:
                solar.unk = br.read_u8();
                solar.unk2 = br.read_u32();
                for (uint32_t i = 0; i < solar.unk2; i++) {
                    solar.unk3.push_back(br.read_u32());
                }
                break;
            case TileExtra::DECO:
                deco.unk = br.read_u8();
                deco.unk2 = br.read_u32();
                deco.unk3 = br.read_u8();
                break;
            case TileExtra::SEWING_MACHINE:
                sewing_machine.unk = br.read_u32();
                break;
            case TileExtra::COUNTRY_FLAG:
                country_flag.flag = br.read_string();
                break;
            case TileExtra::BATTLE_CAGE:
                battle_cage.label = br.read_string();
                battle_cage.pet1 = br.read_u32();
                battle_cage.pet2 = br.read_u32();
                battle_cage.pet3 = br.read_u32();
                break;
            case TileExtra::WEATHER_SPECIAL:
                weather_special.color = br.read_u32();
                break;
            case TileExtra::VIP_ENTRANCE:
                vip_entrance.unk = br.read_u8();
                vip_entrance.owner_id = br.read_u32();
                vip_entrance.access_size = br.read_u32();

                vip_entrance.accesses.reserve(vip_entrance.access_size);

                for (uint32_t i = 0; i < vip_entrance.access_size; i++) {
                    vip_entrance.accesses.push_back(br.read_u32());
                }
                break;
            case TileExtra::GEIGER_CHARGER:
                geiger_charger.unk = br.read_u32();
                break;
            default:
                break;
        }

        // Handle unknown type. (Lazy to name :D)
        switch (static_cast<uint8_t>(type)) {
            case 14: // Mannequin (please move it).
                br.skip_string();
                br.skip(23);
                break;
            case 19: // Dressup (please move it).
                br.skip(18);
                break;
            case 27: // Forge (please move it).
                br.skip(4);
                break;
            case 30: // Steam organ (please move it).
                br.skip(5);
                break;
            case 35:
                br.skip(4);
				br.skip_string();
                break;
            case 39: // Lock-bot (please move it).
                br.skip(4);
                break;
            case 42: // Data bedrock (please move it).
                br.skip(21);
                break;
            case 43: // Display shelf (please move it).
                br.skip(16);
                break;
            case 48: // Potrait (please move it).
                br.skip_string();
                br.skip(26);
                break;
            case 49: // Stuff Weather (please move it).
                br.skip(9);
                break;
            case 55: // Home Oven (please move it).
                br.skip(br.read_u32() * 4);
                br.skip(16);
                break;
            case 56: // Audio Gear, Lucky Token.
                br.skip_string();
                br.skip(4);
                break;
            case 58: // Adventure begins.
                break;
            case 61:
                br.skip(35);
                break;
            case 62:
                br.skip(14);
                break;
            case 63: // Robot
                br.skip(br.read_u32() * 15);
                br.skip(8);
                break;
            case 65: // This is guild item (please move it).
                br.skip(17);
                break;
            case 66: // Growscan9000 (please move it).
                br.skip(1);
                break;
            case 69:
                br.skip(16);
                break;
            case 71: // Sucker2 (please move it).
                br.skip(44);
                break;
            case 72: // Storm Cloud (please move it).
                br.skip(12);
                break;
            case 73:
                br.skip(4);
                break;
            case 74: // Safe Vault
                break;
            case 77: // Infinity Weather Machine. (please move it).
                br.skip(4);
                br.skip(br.read_u32() * 4);
                break;
            case 81: // Friends Entrance. (please move it).
                br.skip(8);
                break;
            default:
                break;
        }

        position = br.position();
    }

    static std::string type_to_string(uint8_t index)
    {
        std::array<std::string, eType::MAX> types{
            "NONE", "DOOR", "SIGN", "LOCK", "SEED", "UNK", "MAILBOX", "BULLETIN", "DICE", "PROVIDER",
            "ACHIEVEMENT", "HEART_MONITOR", "DONATION_BOX", "TOYBOX", "MANNEQUIN", "BUNNY_EGG",
            "GAME_GEN", "UNK2", "XENONITE", "DRESSUP", "CRYSTAL", "BURGLAR", "SPOTLIGHT", "DISPLAY_BLOCK",
            "VENDING", "SOLAR", "UNK3", "UNK4", "DECO", "UNK5", "UNK6", "UNK7", "SEWING_MACHINE",
            "COUNTRY_FLAG", "UNK8", "UNK9", "BATTLE_CAGE", "UNK10", "UNK11", "UNK12", "WEATHER_SPECIAL",
            "UNK13", "UNK14", "UNK15", "VIP_ENTRANCE", "UNK16", "UNK17", "UNK18", "UNK19", "WEATHER_SPECIAL2",
            "UNK20", "UNK21", "HOWLER", "UNK22", "UNK23", "UNK24", "UNK25", "GEIGER_CHARGER", "UNK26", "UNK27",
            "UNK28", "UNK29", "SUCKER", "ROBOT", "UNK30", "UNK31", "GROWSCAN9000", "UNK32", "UNK33", "UNK34",
            "UNK35", "SUCKER2", "UNK36", "UNK27", "SAFE_VAULT"
        };

        if (index > MAX || index < NONE)
            index = NONE;

        return types[index];
    }

    [[nodiscard]] std::string type_to_string() const
    {
        return type_to_string(type);
    }

    [[nodiscard]] std::string get_raw_data() const
    {
        std::string raw_data{ fmt::format("TileExtra::Type -> [{}]:", type_to_string(type)) };
        switch (type) {
            case eType::DOOR:
                return fmt::format("{}\n > label: {}\n > unk: {}", raw_data, door.label, door.unk);
            case eType::SIGN:
                return fmt::format("{}\n > label: {}\n > unk32: {}", raw_data, sign.label, sign.unk);
            case eType::SEED:
                return fmt::format("{}\n > growth_time: {}\n > fruit_count: {}", raw_data, seed.growth_time, seed.fruit_count);
            case eType::PROVIDER:
                return fmt::format("{}\n > unk: {}\n > unk2: {}", raw_data, provider.unk, provider.unk2);
            case eType::HEART_MONITOR:
                return fmt::format("{}\n > unk: {}\n > label: {}", raw_data, heart_monitor.unk, heart_monitor.label);
            default:
                return fmt::format("{}\n > None", raw_data);
        }
    }
};
#pragma pack(pop)
