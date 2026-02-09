#include "tile_extra.hpp"

namespace world::tile_extra {
namespace {
Door serialize_door(utils::ByteStream<>& bs)
{
    Door door{};
    bs.read(door.label);
    bs.read(door.unk);
    return door;
}

Sign serialize_sign(utils::ByteStream<>& bs)
{
    Sign sign{};
    bs.read(sign.label);
    bs.read(sign.unk);
    return sign;
}

Lock serialize_lock(utils::ByteStream<>& bs, const std::uint16_t foreground)
{
    Lock lock{};
    bs.read(lock.unk);
    bs.read(lock.owner_id);

    std::uint32_t access_size{};
    bs.read(access_size);

    lock.accesses.resize(access_size);

    for (std::uint32_t i = 0; i < access_size; i++) {
        bs.read(lock.accesses[i]);
    }

    bs.skip(8);
    if (foreground == 5814) {
        bs.skip(16);
    }

    return lock;
}

Seed serialize_seed(utils::ByteStream<>& bs) {
    Seed seed;
    bs.read(seed.growth_time);
    bs.read(seed.fruit_count);
    return seed;
}

Dice serialize_dice(utils::ByteStream<>& bs) {
    Dice dice;
    bs.read(dice.number);
    return dice;
}

Provider serialize_provider(utils::ByteStream<>& bs, std::uint16_t foreground, std::uint16_t version) {
    Provider provider;
    bs.read(provider.unk);

    if (foreground != 5318 && (foreground != 10656 || version < 17)) {
        return provider;
    }

    bs.read(provider.unk2);
    return provider;
}

Achievement serialize_achievement(utils::ByteStream<>& bs) {
    Achievement achievement;
    bs.read(achievement.unk);
    bs.read(achievement.unk2);
    return achievement;
}

HeartMonitor serialize_heart_monitor(utils::ByteStream<>& bs) {
    HeartMonitor heart_monitor;
    bs.read(heart_monitor.unk);
    bs.read(heart_monitor.label);
    return heart_monitor;
}

BunnyEgg serialize_bunny_egg(utils::ByteStream<>& bs) {
    BunnyEgg bunny_egg;
    bs.read(bunny_egg.unk);
    return bunny_egg;
}

GameGen serialize_game_gen(utils::ByteStream<>& bs) {
    GameGen game_gen;
    bs.read(game_gen.unk);
    return game_gen;
}

Xenonite serialize_xenonite(utils::ByteStream<>& bs) {
    Xenonite xenonite;
    bs.read(xenonite.unk);
    bs.read(xenonite.unk2);
    return xenonite;
}

Crystal serialize_crystal(utils::ByteStream<>& bs) {
    Crystal crystal;
    bs.read(crystal.unk);
    return crystal;
}

Burglar serialize_burglar(utils::ByteStream<>& bs) {
    Burglar burglar;
    bs.read(burglar.unk);
    bs.read(burglar.unk2);
    bs.read(burglar.unk3);
    return burglar;
}

DisplayBlock serialize_display_block(utils::ByteStream<>& bs) {
    DisplayBlock display_block;
    bs.read(display_block.item_id);
    return display_block;
}

Vending serialize_vending(utils::ByteStream<>& bs) {
    Vending vending;
    bs.read(vending.item_id);
    bs.read(vending.price);
    return vending;
}

Solar serialize_solar(utils::ByteStream<>& bs) {
    Solar solar;
    bs.read(solar.unk);
    bs.read(solar.unk2);
    solar.unk3.resize(solar.unk2);
    for (std::uint32_t i = 0; i < solar.unk2; i++) {
        bs.read(solar.unk3[i]);
    }
    return solar;
}

Deco serialize_deco(utils::ByteStream<>& bs) {
    Deco deco;
    bs.read(deco.unk);
    bs.read(deco.unk2);
    bs.read(deco.unk3);
    return deco;
}

SewingMachine serialize_sewing_machine(utils::ByteStream<>& bs) {
    SewingMachine sewing_machine;
    bs.read(sewing_machine.unk);
    return sewing_machine;
}

CountryFlag serialize_country_flag(utils::ByteStream<>& bs) {
    CountryFlag country_flag;
    bs.read(country_flag.flag);
    return country_flag;
}

BattleCage serialize_battle_cage(utils::ByteStream<>& bs) {
    BattleCage battle_cage;
    bs.read(battle_cage.label);
    bs.read(battle_cage.pet1);
    bs.read(battle_cage.pet2);
    bs.read(battle_cage.pet3);
    return battle_cage;
}

WeatherSpecial serialize_weather_special(utils::ByteStream<>& bs) {
    WeatherSpecial weather_special;
    bs.read(weather_special.color);
    return weather_special;
}

VipEntrance serialize_vip_entrance(utils::ByteStream<>& bs) {
    VipEntrance vip_entrance;
    bs.read(vip_entrance.unk);
    bs.read(vip_entrance.owner_id);
    std::uint32_t access_size{};
    bs.read(access_size);
    vip_entrance.accesses.resize(access_size);
    for (std::uint32_t i = 0; i < access_size; i++) {
        bs.read(vip_entrance.accesses[i]);
    }
    return vip_entrance;
}

GeigerCharger serialize_geiger_charger(utils::ByteStream<>& bs) {
    GeigerCharger geiger_charger;
    bs.read(geiger_charger.unk);
    return geiger_charger;
}

// Handle unknown tile types by skipping appropriate number of bytes
void handle_unknown_type(utils::ByteStream<>& bs, std::uint8_t type_u8) {
    switch (type_u8) {
        case 14: {
            std::string tmp;
            bs.read(tmp);
        }
        bs.skip(23);
        break;
        case 19:
            bs.skip(18);
            break;
        case 27:
            bs.skip(4);
            break;
        case 30:
            bs.skip(5);
            break;
        case 35:
            bs.skip(4);
            {
                std::string tmp;
                bs.read(tmp);
            }
            break;
        case 39:
            bs.skip(4);
            break;
        case 42:
            bs.skip(21);
            break;
        case 43:
            bs.skip(16);
            break;
        case 48: {
            std::string tmp;
            bs.read(tmp);
        }
        bs.skip(26);
        break;
        case 49:
            bs.skip(9);
            break;
        case 55: {
            std::uint32_t count;
            bs.read(count);
            bs.skip(count * 4);
        }
        bs.skip(16);
        break;
        case 56: {
            std::string tmp;
            bs.read(tmp);
        }
        bs.skip(4);
        break;
        case 58:
            break;
        case 61:
            bs.skip(35);
            break;
        case 62:
            bs.skip(14);
            break;
        case 63: {
            std::uint32_t count;
            bs.read(count);
            bs.skip(count * 15);
        }
        bs.skip(8);
        break;
        case 65:
            bs.skip(17);
            break;
        case 66:
            bs.skip(1);
            break;
        case 69:
            bs.skip(16);
            break;
        case 71:
            bs.skip(44);
            break;
        case 72:
            bs.skip(12);
            break;
        case 73:
            bs.skip(4);
            break;
        case 74:
            break;
        case 75:
            for (std::uint32_t i = 0; i < 3; i++) {
                bs.skip(104);
            }
            break;
        case 77:
            bs.skip(4);
            {
                std::uint32_t count;
                bs.read(count);
                bs.skip(count * 4);
            }
            break;
        case 81:
            bs.skip(8);
            break;
        default:
            break;
    }
}
}

void TileExtra::serialize(utils::ByteStream<>& bs, std::uint16_t version, std::uint16_t foreground, std::uint16_t background) {
    std::uint8_t type_val{};
    if (!bs.read(type_val)) {
        return;
    }

    type_ = static_cast<Type>(type_val);

    switch (type_) {
        case Type::Door:
            data_ = serialize_door(bs);
            break;
        case Type::Sign:
            data_ = serialize_sign(bs);
            break;
        case Type::Lock:
            data_ = serialize_lock(bs, foreground);
            break;
        case Type::Seed:
            data_ = serialize_seed(bs);
            break;
        case Type::Dice:
            data_ = serialize_dice(bs);
            break;
        case Type::Provider:
            data_ = serialize_provider(bs, foreground, version);
            break;
        case Type::Achievement:
            data_ = serialize_achievement(bs);
            break;
        case Type::HeartMonitor:
            data_ = serialize_heart_monitor(bs);
            break;
        case Type::BunnyEgg:
            data_ = serialize_bunny_egg(bs);
            break;
        case Type::GameGen:
            data_ = serialize_game_gen(bs);
            break;
        case Type::Xenonite:
            data_ = serialize_xenonite(bs);
            break;
        case Type::Crystal:
            data_ = serialize_crystal(bs);
            break;
        case Type::Burglar:
            data_ = serialize_burglar(bs);
            break;
        case Type::DisplayBlock:
            data_ = serialize_display_block(bs);
            break;
        case Type::Vending:
            data_ = serialize_vending(bs);
            break;
        case Type::Solar:
            data_ = serialize_solar(bs);
            break;
        case Type::Deco:
            data_ = serialize_deco(bs);
            break;
        case Type::SewingMachine:
            data_ = serialize_sewing_machine(bs);
            break;
        case Type::CountryFlag:
            data_ = serialize_country_flag(bs);
            break;
        case Type::BattleCage:
            data_ = serialize_battle_cage(bs);
            break;
        case Type::WeatherSpecial:
            data_ = serialize_weather_special(bs);
            break;
        case Type::VipEntrance:
            data_ = serialize_vip_entrance(bs);
            break;
        case Type::GeigerCharger:
            data_ = serialize_geiger_charger(bs);
            break;
        default: {
            const auto type_u8 = static_cast<std::uint8_t>(type_);
            handle_unknown_type(bs, type_u8);
            break;
        }
    }
}
}
