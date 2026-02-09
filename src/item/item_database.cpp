#include "item_database.hpp"

#include <spdlog/spdlog.h>
#include <fstream>

namespace item {
namespace {
const std::string_view DECRYPT_KEY = "PBG892FXX982ABC*";
}

std::string ItemDatabase::decrypt_item_name(std::string_view encrypted, std::uint32_t item_id)
{
    std::string decrypted;
    decrypted.reserve(encrypted.size());

    for (std::size_t i = 0; i < encrypted.size(); ++i) {
        const std::size_t key_index = (i + item_id) % DECRYPT_KEY.size();
        decrypted += static_cast<char>(encrypted[i] ^ DECRYPT_KEY[key_index]);
    }

    return decrypted;
}

std::optional<ItemInfo> ItemDatabase::parse_item(utils::ByteStream<>& bs) const
{
    ItemInfo item{};

    bs.read(item.item_id);
    bs.read(item.flags);
    bs.read(item.item_type);
    bs.read(item.material);

    bs.read(item.item_name);
    item.item_name = decrypt_item_name(item.item_name, item.item_id);

    bs.read(item.texture_file_path);
    bs.read(item.texture_file_hash);

    bs.read(item.visual_effect);
    bs.read(item.cooking_time);

    bs.read(item.texture_coord_x);
    bs.read(item.texture_coord_y);

    bs.read(item.texture_type);

    bs.read(item.is_stripey_wallpaper);

    bs.read(item.collision_type);

    bs.read(item.health);
    bs.read(item.reset_time);

    bs.read(item.clothing_type);

    bs.read(item.rarity);
    bs.read(item.max_amount);

    bs.read(item.extra_file);
    bs.read(item.extra_file_hash);

    bs.read(item.animation_time);

    bs.read(item.pet_name);
    bs.read(item.pet_prefix);
    bs.read(item.pet_suffix);
    bs.read(item.pet_ability);

    bs.read(item.seed_base);
    bs.read(item.seed_overlay);
    bs.read(item.tree_base);
    bs.read(item.tree_leaves);
    bs.read(item.seed_color);
    bs.read(item.seed_overlay_color);
    bs.read(item.ingredient);
    bs.read(item.grow_time);

    bs.read(item.fx_flag);

    bs.read(item.animating_coordinates);
    bs.read(item.animating_texture_file);
    bs.read(item.animating_coordinates_2);

    bs.read(item.__unk_1);
    bs.read(item.__unk_2);

    bs.read(item.flags_2);

    bs.read(item.__unk_3);

    bs.read(item.tile_range);

    bs.read(item.vault_capacity);

    bs.read(item.punch_options);

    bs.read(item.__unk_4);

    bs.read(item.body_part_list);

    bs.read(item.light_range);

    bs.read(item.__unk_5);

    bs.read(item.can_sit);
    bs.read(item.player_offset_x);
    bs.read(item.player_offset_y);
    bs.read(item.chair_texture_x);
    bs.read(item.chair_texture_y);
    bs.read(item.chair_leg_offset_x);
    bs.read(item.chair_leg_offset_y);
    bs.read(item.chair_texture_file);
    bs.read(item.renderer_data_file);

    bs.read(item.__unk_6);
    bs.read(item.renderer_data_file_hash);

    bs.read(item.__unk_7);
    bs.read(item.__unk_8);

    bs.read(item.info);

    bs.read(item.ingredients);

    if (version_ >= 24) {
        bs.read(item.__unk_9);
    }

    return item;
}

bool ItemDatabase::load_from_file(const std::string& file_path)
{
    std::ifstream file{ file_path, std::ios::binary | std::ios::ate };
    if (!file) {
        spdlog::error("Failed to open items.dat: {}", file_path);
        return false;
    }

    const auto file_size{ file.tellg() };
    file.seekg(0, std::ios::beg);

    std::vector<std::byte> buffer(file_size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), file_size)) {
        spdlog::error("Failed to read items.dat: {}", file_path);
        return false;
    }

    return parse(buffer);
}

bool ItemDatabase::parse(std::span<const std::byte> data)
{
    clear();

    utils::ByteStream<> bs{ data };

    if (!bs.read(version_) || !bs.read(count_)) {
        spdlog::error("Failed to read items.dat header");
        return false;
    }

    spdlog::info("Parsing items.dat: version={}, count={}", version_, count_);

    if (version_ > 24) {
        spdlog::warn("Unsupported items.dat version: {}, max supported is 24", version_);
        // Don't fail, just warn - parsing might still work for newer versions
    }

    items_.reserve(count_);

    for (std::uint32_t i = 0; i < count_; ++i) {
        auto item_result{ parse_item(bs) };
        if (!item_result.has_value()) {
            spdlog::error("Failed to parse item at index {}", i);
            return false;
        }

        auto& item{ item_result.value() };
        if (item.item_id != i) {
            spdlog::warn("Item ID mismatch at index {}: expected {}, got {}", i, i, item.item_id);
        }

        items_.push_back(std::move(item));
    }

    spdlog::info("Successfully parsed {} items from items.dat", items_.size());
    return true;
}

const ItemInfo* ItemDatabase::get_item(std::uint32_t id) const
{
    if (id >= items_.size()) {
        return nullptr;
    }

    return &items_[id];
}

void ItemDatabase::clear()
{
    version_ = 0;
    count_ = 0;
    items_.clear();
}
}
