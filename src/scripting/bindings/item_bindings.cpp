#include "item_bindings.hpp"

#include <magic_enum/magic_enum.hpp>
#include <sol/sol.hpp>

#include "../../item/item_database.hpp"
#include "../../item/item_info.hpp"

namespace scripting::bindings {
void ItemBindings::bind_item_database(sol::state& lua)
{
    lua.new_usertype<item::ItemDatabase>("ItemDatabase",
        sol::no_constructor,
        "get_version", &item::ItemDatabase::get_version,
        "get_count", &item::ItemDatabase::get_count,
        "empty", &item::ItemDatabase::empty,
        "get_item", [](const item::ItemDatabase& db, std::uint32_t id) -> const item::ItemInfo* {
            return db.get_item(id);
        }
    );

    lua["item_database"] = std::ref(item::ItemDatabase::instance());
}

void ItemBindings::bind_item_info(sol::state& lua)
{
    lua.new_usertype<item::ItemInfo>("ItemInfo",
        sol::no_constructor,
        "item_id", &item::ItemInfo::item_id,
        "item_name", &item::ItemInfo::item_name,
        "item_type", &item::ItemInfo::item_type,
        "material", &item::ItemInfo::material,
        "texture_file_path", &item::ItemInfo::texture_file_path,
        "visual_effect", &item::ItemInfo::visual_effect,
        "collision_type", &item::ItemInfo::collision_type,
        "clothing_type", &item::ItemInfo::clothing_type,
        "rarity", &item::ItemInfo::rarity,
        "max_amount", &item::ItemInfo::max_amount,
        "health", &item::ItemInfo::health,
        "flags", sol::property([](const item::ItemInfo& info, sol::this_state s) {
            sol::state_view lua{ s };
            sol::table flags{ lua.create_table() };
            flags["flippable"] = static_cast<bool>(info.flags.flippable);
            flags["editable"] = static_cast<bool>(info.flags.editable);
            flags["seedless"] = static_cast<bool>(info.flags.seedless);
            flags["permanent"] = static_cast<bool>(info.flags.permanent);
            flags["dropless"] = static_cast<bool>(info.flags.dropless);
            flags["no_self"] = static_cast<bool>(info.flags.no_self);
            flags["no_shadow"] = static_cast<bool>(info.flags.no_shadow);
            flags["world_locked"] = static_cast<bool>(info.flags.world_locked);
            flags["beta"] = static_cast<bool>(info.flags.beta);
            flags["auto_pickup"] = static_cast<bool>(info.flags.auto_pickup);
            flags["mod"] = static_cast<bool>(info.flags.mod);
            flags["random_grow"] = static_cast<bool>(info.flags.random_grow);
            flags["is_public"] = static_cast<bool>(info.flags.is_public);
            flags["foreground"] = static_cast<bool>(info.flags.foreground);
            flags["holiday"] = static_cast<bool>(info.flags.holiday);
            flags["untradeable"] = static_cast<bool>(info.flags.untradeable);
            return flags;
        })
    );
}

void ItemBindings::bind_enums(sol::state& lua)
{
    auto item_table = lua["item"].get_or_create<sol::table>();

    auto type_table{ lua.create_table() };
    for (const auto v : magic_enum::enum_values<item::ItemType>()) {
        type_table[magic_enum::enum_name(v)] = static_cast<std::uint8_t>(v);
    }
    item_table["Type"] = type_table;

    auto collision_table{ lua.create_table() };
    for (const auto v : magic_enum::enum_values<item::CollisionType>()) {
        collision_table[magic_enum::enum_name(v)] = static_cast<std::uint8_t>(v);
    }
    item_table["CollisionType"] = collision_table;

    auto visual_table{ lua.create_table() };
    for (const auto v : magic_enum::enum_values<item::VisualEffect>()) {
        visual_table[magic_enum::enum_name(v)] = static_cast<std::uint8_t>(v);
    }
    item_table["VisualEffect"] = visual_table;

    auto material_table{ lua.create_table() };
    for (const auto v : magic_enum::enum_values<item::MaterialType>()) {
        material_table[magic_enum::enum_name(v)] = static_cast<std::uint8_t>(v);
    }
    item_table["MaterialType"] = material_table;

    auto clothing_table{ lua.create_table() };
    for (const auto v : magic_enum::enum_values<item::ClothingType>()) {
        clothing_table[magic_enum::enum_name(v)] = static_cast<std::uint8_t>(v);
    }
    item_table["ClothingType"] = clothing_table;
}

void ItemBindings::bind(sol::state& lua)
{
    auto item_table{ lua.create_table() };

    bind_item_database(lua);
    bind_item_info(lua);
    bind_enums(lua);
}
}
