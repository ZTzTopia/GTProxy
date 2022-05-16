#pragma once
#include <cstdint>
#include <string>

#include "../utils/binary_reader.h"

namespace items {
    struct ItemInfo {
        uint32_t id;
        uint8_t editable_type;
        uint8_t category;
        uint8_t action_type;
        uint8_t hit_sound_type;
        uint16_t name_length;
        std::string name;
        uint16_t texture_length;
        std::string texture;
        uint32_t texture_hash;
        uint8_t kind;
        uint32_t flags;
        uint8_t texture_pos_x;
        uint8_t texture_pos_y;
        uint8_t spread_type;
        uint8_t stripey_wallpaper;
        uint8_t collision_type;
        uint8_t break_hits;
        uint32_t reset_time;
        uint8_t clothing_type;
        uint16_t rarity;
        uint8_t max_amount;
        uint16_t extra_file_length;
        std::string extra_file;
        uint32_t extra_file_hash;
        uint32_t audio_volume;
        uint16_t pet_name_length;
        std::string pet_name;
        uint16_t pet_prefix_length;
        std::string pet_prefix;
        uint16_t pet_suffix_length;
        std::string pet_suffix;
        uint16_t pet_ability_length;
        std::string pet_ability;
        uint8_t seed_base;
        uint8_t seed_overlay;
        uint8_t tree_base;
        uint8_t tree_leaves;
        uint32_t seed_color;
        uint32_t seed_overlay_color;
        uint32_t seed_ingredient;
        uint32_t seed_grow_time;
        uint16_t flags2;
        uint16_t is_rayman;
        uint16_t extra_option_length;
        std::string extra_option;
        uint16_t texture2_length;
        std::string texture2;
        uint16_t extra_option2_length;
        std::string extra_option2;
        uint8_t pad[80];
        uint16_t punch_option_length;
        std::string punch_option;
        uint32_t flags3;
        uint8_t body_part[9];
        uint32_t flags4;
        uint32_t flags5;

        static std::string cypher(const std::string &input, uint32_t id) {
            constexpr std::string_view key = "PBG892FXX982ABC*";
            std::string return_value(input.size(), 0);

            for (uint32_t i = 0; i < input.size(); i++)
                return_value[i] = static_cast<char>(input[i] ^ key[(i + id) % key.size()]);

            return return_value;
        }

        void serialize(void* buffer, uint16_t version, std::size_t& position) {
            BinaryReader binary_reader{ buffer };
            binary_reader.skip(position);

            id = binary_reader.read_u32();
            editable_type = binary_reader.read_u8();
            category = binary_reader.read_u8();
            action_type = binary_reader.read_u8();
            hit_sound_type = binary_reader.read_u8();
            name = cypher(binary_reader.read_string(), id);
            texture = binary_reader.read_string();
            texture_hash = binary_reader.read_u32();
            kind = binary_reader.read_u8();
            flags = binary_reader.read_u32();
            texture_pos_x = binary_reader.read_u8();
            texture_pos_y = binary_reader.read_u8();
            spread_type = binary_reader.read_u8();
            stripey_wallpaper = binary_reader.read_u8();
            collision_type = binary_reader.read_u8();
            break_hits = binary_reader.read_u8();
            reset_time = binary_reader.read_u32();
            clothing_type = binary_reader.read_u8();
            rarity = binary_reader.read_u16();
            max_amount = binary_reader.read_u8();
            extra_file = binary_reader.read_string();
            extra_file_hash = binary_reader.read_u32();
            audio_volume = binary_reader.read_u32();
            pet_name = binary_reader.read_string();
            pet_prefix = binary_reader.read_string();
            pet_suffix = binary_reader.read_string();
            pet_ability = binary_reader.read_string();
            seed_base = binary_reader.read_u8();
            seed_overlay = binary_reader.read_u8();
            tree_base = binary_reader.read_u8();
            tree_leaves = binary_reader.read_u8();
            seed_color = binary_reader.read_u32();
            seed_overlay_color = binary_reader.read_u32();
            seed_ingredient = binary_reader.read_u32();
            seed_grow_time = binary_reader.read_u32();
            flags2 = binary_reader.read_u16();
            is_rayman = binary_reader.read_u16();
            extra_option = binary_reader.read_string();
            texture2 = binary_reader.read_string();
            extra_option2 = binary_reader.read_string();
            binary_reader.skip(80);

            if (version > 10) {
                punch_option = binary_reader.read_string();
            }

            if (version > 11) {
                flags3 = binary_reader.read_u32();
                binary_reader.skip(9); // Body part.
            }

            if (version > 12) {
                binary_reader.skip(4); // unk
            }

            if (version > 13) {
                binary_reader.skip(4); // unk
            }

            position = binary_reader.position();
        }
    };
}
