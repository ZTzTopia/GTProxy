#pragma once
#include <cstdint>
#include <string>

#include "tile_extra.hpp"
#include <fmt/format.h>

namespace world {
enum class TileFlag : std::uint16_t {
    None = 0,
    Extra = 1 << 0,
    Locked = 1 << 1,
    Seed = 1 << 4,
    Flipped = 1 << 5,
    Open = 1 << 6,
    Public = 1 << 7,
    Silenced = 1 << 9,
    Water = 1 << 10,
    Glue = 1 << 11,
    Fire = 1 << 12,
    Red = 1 << 13,
    Green = 1 << 14,
    Blue = 1 << 15
};

[[nodiscard]] inline TileFlag operator|(TileFlag lhs, TileFlag rhs) {
    return static_cast<TileFlag>(static_cast<std::uint16_t>(lhs) | static_cast<std::uint16_t>(rhs));
}

[[nodiscard]] inline TileFlag operator&(TileFlag lhs, TileFlag rhs) {
    return static_cast<TileFlag>(static_cast<std::uint16_t>(lhs) & static_cast<std::uint16_t>(rhs));
}

[[nodiscard]] inline TileFlag operator~(TileFlag flag) {
    return static_cast<TileFlag>(~static_cast<std::uint16_t>(flag));
}

[[nodiscard]] inline TileFlag& operator|=(TileFlag& lhs, TileFlag rhs) {
    lhs = lhs | rhs;
    return lhs;
}

[[nodiscard]] inline bool has_flag(TileFlag flags, TileFlag flag) {
    return (flags & flag) == flag;
}

struct Tile {
    std::uint16_t foreground;
    std::uint16_t background;
    std::uint16_t parent_tile;
    TileFlag flag;
    std::uint16_t lock_parent_tile;
    tile_extra::TileExtra extra;

    Tile()
        : foreground{ 0 }
        , background{ 0 }
        , parent_tile{ 0 }
        , flag{ TileFlag::None }
        , lock_parent_tile{ 0 }
    {

    }

    void serialize(utils::ByteStream<>& bs, const std::uint16_t version)
    {
        bs.read(foreground);
        bs.read(background);
        bs.read(parent_tile);

        std::uint16_t flag_val{};
        bs.read(flag_val);
        flag = static_cast<TileFlag>(flag_val);

        if (parent_tile) {
            bs.read(lock_parent_tile);
        }

        if (has_flag(flag, TileFlag::Extra)) {
            extra.serialize(bs, version, foreground, background);
        }
    }

    [[nodiscard]] std::string flag_to_string() const
    {
        if (flag == TileFlag::None) {
            return "None";
        }

        std::string flag_string;
        
        if (has_flag(flag, TileFlag::Extra)) flag_string.append("Extra|");
        if (has_flag(flag, TileFlag::Locked)) flag_string.append("Locked|");
        if (has_flag(flag, TileFlag::Seed)) flag_string.append("Seed|");
        if (has_flag(flag, TileFlag::Flipped)) flag_string.append("Flipped|");
        if (has_flag(flag, TileFlag::Open)) flag_string.append("Open|");
        if (has_flag(flag, TileFlag::Public)) flag_string.append("Public|");
        if (has_flag(flag, TileFlag::Silenced)) flag_string.append("Silenced|");
        if (has_flag(flag, TileFlag::Water)) flag_string.append("Water|");
        if (has_flag(flag, TileFlag::Glue)) flag_string.append("Glue|");
        if (has_flag(flag, TileFlag::Fire)) flag_string.append("Fire|");
        if (has_flag(flag, TileFlag::Red)) flag_string.append("Red|");
        if (has_flag(flag, TileFlag::Green)) flag_string.append("Green|");
        if (has_flag(flag, TileFlag::Blue)) flag_string.append("Blue|");

        if (!flag_string.empty()) {
            flag_string.pop_back();
        }

        return flag_string;
    }

    [[nodiscard]] std::string get_raw_data() const {
        if (!has_flag(flag, TileFlag::Extra)) {
            return fmt::format("Tile::Type -> [{}]", flag_to_string());
        }

        return fmt::format("Tile::Type -> [{}]:\n{}", flag_to_string(), extra.get_raw_data());
    }
};
}
