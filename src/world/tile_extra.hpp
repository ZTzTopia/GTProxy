#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include <memory>

#include "../utils/byte_stream.hpp"
#include <fmt/format.h>

namespace world::tile_extra {

struct Door {
    std::string label;
    std::uint8_t unk;
};

struct Sign {
    std::string label;
    std::uint32_t unk;
};

struct Lock {
    std::uint32_t unk;
    std::uint32_t owner_id;
    std::vector<std::uint32_t> accesses;
};

struct Seed {
    std::uint32_t growth_time;
    std::uint8_t fruit_count;
};

struct Dice {
    std::uint8_t number;
};

struct Provider {
    std::uint32_t unk;
    std::uint32_t unk2;
};

struct Achievement {
    std::uint32_t unk;
    std::uint8_t unk2;
};

struct HeartMonitor {
    std::uint32_t unk;
    std::string label;
};

struct BunnyEgg {
    std::uint32_t unk;
};

struct GameGen {
    std::uint8_t unk;
};

struct Xenonite {
    std::uint8_t unk;
    std::uint32_t unk2;
};

struct Crystal {
    std::string unk;
};

struct Burglar {
    std::string unk;
    std::uint32_t unk2;
    std::uint8_t unk3;
};

struct DisplayBlock {
    std::uint32_t item_id;
};

struct Vending {
    std::uint32_t item_id;
    std::uint32_t price;
};

struct Solar {
    std::uint8_t unk;
    std::uint32_t unk2;
    std::vector<std::uint32_t> unk3;
};

struct Deco {
    std::uint8_t unk;
    std::uint32_t unk2;
    std::uint8_t unk3;
};

struct SewingMachine {
    std::uint32_t unk;
};

struct CountryFlag {
    std::string flag;
};

struct BattleCage {
    std::string label;
    std::uint32_t pet1;
    std::uint32_t pet2;
    std::uint32_t pet3;
};

struct WeatherSpecial {
    std::uint32_t color;
};

struct VipEntrance {
    std::uint8_t unk;
    std::uint32_t owner_id;
    std::vector<std::uint32_t> accesses;
};

struct GeigerCharger {
    std::uint32_t unk;
};

struct Unknown {
    std::vector<std::uint8_t> data;
};

enum class Type : std::uint8_t {
    None,
    Door,
    Sign,
    Lock,
    Seed,
    Mailbox,
    Bulletin,
    Dice,
    Provider,
    Achievement,
    HeartMonitor,
    DonationBox,
    Toybox,
    Mannequin,
    BunnyEgg,
    GameGen,
    Xenonite,
    Dressup,
    Crystal,
    Burglar,
    Spotlight,
    DisplayBlock,
    Vending,
    Solar,
    Deco,
    SewingMachine,
    CountryFlag,
    BattleCage,
    WeatherSpecial,
    VipEntrance,
    Howler,
    GeigerCharger,
    Sucker,
    Robot,
    Growscan9000,
    Sucker2,
    SafeVault
};

using Variant = std::variant<std::monostate, Door, Sign, Lock, Seed, Dice, Provider, Achievement, HeartMonitor, BunnyEgg, GameGen, Xenonite, Crystal, Burglar, DisplayBlock, Vending, Solar, Deco, SewingMachine, CountryFlag, BattleCage, WeatherSpecial, VipEntrance, GeigerCharger, Unknown>;

class TileExtra {
public:
    Type type_;
    Variant data_;

    TileExtra()
        : type_{ Type::None }
        , data_{ std::monostate{} }
    {
    }

    ~TileExtra() = default;

    [[nodiscard]] Type get_type() const
    {
        return type_;
    }

    [[nodiscard]] bool has_value() const
    {
        return !std::holds_alternative<std::monostate>(data_);
    }

    [[nodiscard]] std::string get_raw_data() const
    {
        if (!has_value()) {
            return "TileExtra::Type -> [None]";
        }

        const Type current_type = type_;
        return std::visit([current_type](const auto& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            
            if constexpr (std::is_same_v<T, std::monostate>) {
                return "TileExtra::Type -> [None]";
            } else if constexpr (std::is_same_v<T, Door>) {
                return fmt::format("TileExtra::Type -> [Door]:\n > label: {}\n > unk: {}", arg.label, arg.unk);
            } else if constexpr (std::is_same_v<T, Sign>) {
                return fmt::format("TileExtra::Type -> [Sign]:\n > label: {}\n > unk32: {}", arg.label, arg.unk);
            } else if constexpr (std::is_same_v<T, Seed>) {
                return fmt::format("TileExtra::Type -> [Seed]:\n > growth_time: {}\n > fruit_count: {}", arg.growth_time, arg.fruit_count);
            } else if constexpr (std::is_same_v<T, Provider>) {
                return fmt::format("TileExtra::Type -> [Provider]:\n > unk: {}\n > unk2: {}", arg.unk, arg.unk2);
            } else if constexpr (std::is_same_v<T, HeartMonitor>) {
                return fmt::format("TileExtra::Type -> [HeartMonitor]:\n > unk: {}\n > label: {}", arg.unk, arg.label);
            } else {
                return fmt::format("TileExtra::Type -> [{}]: (Data not displayed)", type_to_string(current_type));
            }
        }, data_);
    }

    void serialize(utils::ByteStream<>& bs, std::uint16_t version, std::uint16_t foreground, std::uint16_t background)
    {
        std::uint8_t type_val{};
        if (!bs.read(type_val)) {
            return;
        }

        type_ = static_cast<Type>(type_val);

        switch (type_) {
            case Type::Door: {
                Door door;
                bs.read(door.label);
                bs.read(door.unk);
                data_ = door;
                break;
            }
            case Type::Sign: {
                Sign sign;
                bs.read(sign.label);
                bs.read(sign.unk);
                data_ = sign;
                break;
            }
            case Type::Lock: {
                Lock lock;
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
                data_ = lock;
                break;
            }
            case Type::Seed: {
                Seed seed;
                bs.read(seed.growth_time);
                bs.read(seed.fruit_count);
                data_ = seed;
                break;
            }
            case Type::Dice: {
                Dice dice;
                bs.read(dice.number);
                data_ = dice;
                break;
            }
            case Type::Provider: {
                Provider provider;
                bs.read(provider.unk);

                if (foreground != 5318 && (foreground != 10656 || version < 17)) {
                    data_ = provider;
                    break;
                }

                bs.read(provider.unk2);
                data_ = provider;
                break;
            }
            case Type::Achievement: {
                Achievement achievement;
                bs.read(achievement.unk);
                bs.read(achievement.unk2);
                data_ = achievement;
                break;
            }
            case Type::HeartMonitor: {
                HeartMonitor heart_monitor;
                bs.read(heart_monitor.unk);
                bs.read(heart_monitor.label);
                data_ = heart_monitor;
                break;
            }
            case Type::BunnyEgg: {
                BunnyEgg bunny_egg;
                bs.read(bunny_egg.unk);
                data_ = bunny_egg;
                break;
            }
            case Type::GameGen: {
                GameGen game_gen;
                bs.read(game_gen.unk);
                data_ = game_gen;
                break;
            }
            case Type::Xenonite: {
                Xenonite xenonite;
                bs.read(xenonite.unk);
                bs.read(xenonite.unk2);
                data_ = xenonite;
                break;
            }
            case Type::Crystal: {
                Crystal crystal;
                bs.read(crystal.unk);
                data_ = crystal;
                break;
            }
            case Type::Burglar: {
                Burglar burglar;
                bs.read(burglar.unk);
                bs.read(burglar.unk2);
                bs.read(burglar.unk3);
                data_ = burglar;
                break;
            }
            case Type::DisplayBlock: {
                DisplayBlock display_block;
                bs.read(display_block.item_id);
                data_ = display_block;
                break;
            }
            case Type::Vending: {
                Vending vending;
                bs.read(vending.item_id);
                bs.read(vending.price);
                data_ = vending;
                break;
            }
            case Type::Solar: {
                Solar solar;
                bs.read(solar.unk);
                bs.read(solar.unk2);
                solar.unk3.resize(solar.unk2);
                for (std::uint32_t i = 0; i < solar.unk2; i++) {
                    bs.read(solar.unk3[i]);
                }
                data_ = solar;
                break;
            }
            case Type::Deco: {
                Deco deco;
                bs.read(deco.unk);
                bs.read(deco.unk2);
                bs.read(deco.unk3);
                data_ = deco;
                break;
            }
            case Type::SewingMachine: {
                SewingMachine sewing_machine;
                bs.read(sewing_machine.unk);
                data_ = sewing_machine;
                break;
            }
            case Type::CountryFlag: {
                CountryFlag country_flag;
                bs.read(country_flag.flag);
                data_ = country_flag;
                break;
            }
            case Type::BattleCage: {
                BattleCage battle_cage;
                bs.read(battle_cage.label);
                bs.read(battle_cage.pet1);
                bs.read(battle_cage.pet2);
                bs.read(battle_cage.pet3);
                data_ = battle_cage;
                break;
            }
            case Type::WeatherSpecial: {
                WeatherSpecial weather_special;
                bs.read(weather_special.color);
                data_ = weather_special;
                break;
            }
            case Type::VipEntrance: {
                VipEntrance vip_entrance;
                bs.read(vip_entrance.unk);
                bs.read(vip_entrance.owner_id);
                std::uint32_t access_size{};
                bs.read(access_size);
                vip_entrance.accesses.resize(access_size);
                for (std::uint32_t i = 0; i < access_size; i++) {
                    bs.read(vip_entrance.accesses[i]);
                }
                data_ = vip_entrance;
                break;
            }
            case Type::GeigerCharger: {
                GeigerCharger geiger_charger;
                bs.read(geiger_charger.unk);
                data_ = geiger_charger;
                break;
            }
            default: {
                Unknown unknown;
                const auto start_pos = bs.get_read_offset();

                const auto type_u8 = static_cast<std::uint8_t>(type_);
                switch (type_u8) {
                    case 14:
                        {
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
                    case 48:
                        {
                            std::string tmp;
                            bs.read(tmp);
                        }
                        bs.skip(26);
                        break;
                    case 49:
                        bs.skip(9);
                        break;
                    case 55:
                        {
                            std::uint32_t count;
                            bs.read(count);
                            bs.skip(count * 4);
                        }
                        bs.skip(16);
                        break;
                    case 56:
                        {
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
                    case 63:
                        {
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
                        for (std::uint32_t i = 0; i < 3; i++)
                            bs.skip(104);
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

                const auto end_pos = bs.get_read_offset();
                const auto size = end_pos - start_pos;
                unknown.data.resize(size);

                bs.backtrack(size);
                bs.read(unknown.data);
                bs.skip(size);

                data_ = unknown;
                break;
            }
        }
    }

    template <typename T>
    [[nodiscard]] T* get_as() { return std::get_if<T>(&data_); }

    template <typename T>
    [[nodiscard]] const T* get_as() const { return std::get_if<T>(&data_); }

private:
    static std::string type_to_string(Type type) {
        switch (type) {
            case Type::None: return "None";
            case Type::Door: return "Door";
            case Type::Sign: return "Sign";
            case Type::Lock: return "Lock";
            case Type::Seed: return "Seed";
            case Type::Mailbox: return "Mailbox";
            case Type::Bulletin: return "Bulletin";
            case Type::Dice: return "Dice";
            case Type::Provider: return "Provider";
            case Type::Achievement: return "Achievement";
            case Type::HeartMonitor: return "HeartMonitor";
            case Type::DonationBox: return "DonationBox";
            case Type::Toybox: return "Toybox";
            case Type::Mannequin: return "Mannequin";
            case Type::BunnyEgg: return "BunnyEgg";
            case Type::GameGen: return "GameGen";
            case Type::Xenonite: return "Xenonite";
            case Type::Dressup: return "Dressup";
            case Type::Crystal: return "Crystal";
            case Type::Burglar: return "Burglar";
            case Type::Spotlight: return "Spotlight";
            case Type::DisplayBlock: return "DisplayBlock";
            case Type::Vending: return "Vending";
            case Type::Solar: return "Solar";
            case Type::Deco: return "Deco";
            case Type::SewingMachine: return "SewingMachine";
            case Type::CountryFlag: return "CountryFlag";
            case Type::BattleCage: return "BattleCage";
            case Type::WeatherSpecial: return "WeatherSpecial";
            case Type::VipEntrance: return "VipEntrance";
            case Type::Howler: return "Howler";
            case Type::GeigerCharger: return "GeigerCharger";
            case Type::Sucker: return "Sucker";
            case Type::Robot: return "Robot";
            case Type::Growscan9000: return "Growscan9000";
            case Type::Sucker2: return "Sucker2";
            case Type::SafeVault: return "SafeVault";
            default: return "Unknown";
        }
    }
};
}
