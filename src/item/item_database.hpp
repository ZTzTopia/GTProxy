#pragma once
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "item_info.hpp"
#include "../utils/byte_stream.hpp"
#include "../utils/singleton.hpp"

namespace item {
class ItemDatabase : public utils::Singleton<ItemDatabase> {
public:
    [[nodiscard]] bool load_from_file(const std::string& file_path);

    [[nodiscard]] bool parse(std::span<const std::byte> data);

    [[nodiscard]] const ItemInfo* get_item(std::uint32_t id) const;
    [[nodiscard]] std::uint16_t get_version() const noexcept { return version_; }
    [[nodiscard]] std::uint32_t get_count() const noexcept { return count_; }
    [[nodiscard]] bool empty() const noexcept { return items_.empty(); }

    void clear();

private:
    [[nodiscard]] static std::string decrypt_item_name(std::string_view encrypted, std::uint32_t item_id);

    [[nodiscard]] std::optional<ItemInfo> parse_item(utils::ByteStream<>& bs) const;

private:
    std::uint16_t version_{ 0 };
    std::uint32_t count_{ 0 };
    std::vector<ItemInfo> items_;
};
}
