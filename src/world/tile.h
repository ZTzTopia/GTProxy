#pragma once
#include <cstdint>
#include <array>

#include "tile_extra.h"

#pragma pack(push, 1)
struct Tile {
    enum eFlag : uint16_t {
        NONE = 0,
        EXTRA = 1 << 0,
        LOCKED = 1 << 1,
        SEED = 1 << 4,
        FLIPPED = 1 << 5,
        OPEN = 1 << 6,
        PUBLIC = 1 << 7,
        SILENCED = 1 << 9,
        WATER = 1 << 10,
        GLUE = 1 << 11,
        FIRE = 1 << 12,
        RED = 1 << 13,
        GREEN = 1 << 14,
        BLUE = 1 << 15,
        MAX = 15
    };

    uint16_t foreground;
    uint16_t background;
    uint16_t parent_tile;
    eFlag flag;
    uint16_t lock_parent_tile;
    TileExtra tile_extra;

    Tile()
        : foreground(0), background(0), parent_tile(0), flag(NONE), lock_parent_tile(0), tile_extra() {}
    ~Tile() = default;

    static std::string flag_to_string(eFlag flag)
    {
        if (flag == 0)
            return "NONE";

        std::array<std::string, eFlag::MAX> flags{
            "EXTRA", "LOCKED", "UNK", "UNK2" "SEED", "FLIPPED", "OPEN", "PUBLIC", "UNK3", "SILENCED", "WATER",
            "GLUE", "FIRE", "RED", "GREEN", "BLUE"
        };

        std::string flag_string;
        for (int i = 0; i < flags.size(); i++) {
            if (flag & (1 << i)) {
                flag_string.append(flags[i]);
                flag_string.push_back('|');
            }
        }

        flag_string.pop_back();
        return flag_string;
    }

    [[nodiscard]] std::string flag_to_string() const
    {
        return flag_to_string(flag);
    }

    void serialize(void* buffer, uint16_t version) {
        std::size_t temp{ 0 };
        serialize(buffer, temp, version);
    }

    void serialize(void* buffer, std::size_t& position, uint16_t version) {
        BinaryReader br{ buffer };
        br.skip(position);

        foreground = br.read_u16();
        background = br.read_u16();
        parent_tile = br.read_u16();
        flag = static_cast<Tile::eFlag>(br.read_u16());
        if (parent_tile)
            lock_parent_tile = br.read_u16();

        position = br.position();
        if ((flag & Tile::EXTRA) == Tile::EXTRA)
            tile_extra.serialize(buffer, position, version, std::make_pair(foreground, background));
    }

    [[nodiscard]] std::string get_raw_data() const
    {
        if ((flag & Tile::EXTRA) != Tile::EXTRA)
            return fmt::format("Tile::Type -> [{}]", flag_to_string());

        return fmt::format("Tile::Type -> [{}]:\n{}", flag_to_string(), tile_extra.get_raw_data());
    }
};
#pragma pack(pop)
