#pragma once
#include <string>
#include <variant>
#include <vector>
#include <fmt/format.h>
#include <magic_enum/magic_enum.hpp>

#include "../utils/byte_stream.hpp"

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
    std::uint8_t unk;
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
    Pad, // Unknown?
    Dice,
    Provider,
    Achievement,
    HeartMonitor,
    DonationBox,
    Toybox,
    Mannequin,
    BunnyEgg,
    Pad2, // Game Grave?
    GameGen,
    Xenonite,
    Dressup,
    Crystal,
    Burglar,
    Spotlight,
    DisplayBlock,
    Vending,
    Solar,
    Pad3, // Fish Tank Port?
    Pad4, // Forge?
    Deco,
    Pad5, // Giving Tree Stump
    Pad6, // Steam Organ?
    Pad7, // Silk Worm?
    SewingMachine,
    CountryFlag,
    Pad8, // Lobster Trap?
    Pad9, // Painting Essel?
    BattleCage,
    Pad10, // Pet Trainer?
    WeatherSpecial = 40,
    VipEntrance = 44,
    Howler = 52,
    GeigerCharger = 57,
    Sucker = 62,
    Robot,
    Growscan9000 = 66,
    Sucker2 = 71,
    SafeVault = 74
};

using Variant = std::variant<
    std::monostate, Door, Sign, Lock, Seed, Dice, Provider, Achievement, HeartMonitor, BunnyEgg, GameGen, Xenonite,
    Crystal, Burglar, DisplayBlock, Vending, Solar, Deco, SewingMachine, CountryFlag, BattleCage, WeatherSpecial,
    VipEntrance, GeigerCharger, Unknown
>;

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

    void serialize(utils::ByteStream<>& bs, std::uint16_t version, std::uint16_t foreground, std::uint16_t background);

public:
    [[nodiscard]] Type get_type() const { return type_; }
    [[nodiscard]] bool has_value() const { return !std::holds_alternative<std::monostate>(data_); }

    template <typename T>
    [[nodiscard]] T* get_as() { return std::get_if<T>(&data_); }

    template <typename T>
    [[nodiscard]] const T* get_as() const { return std::get_if<T>(&data_); }
};
}
