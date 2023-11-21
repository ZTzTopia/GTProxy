#pragma once
#include <string_view>

namespace hash {
[[nodiscard]] constexpr std::uint32_t fnv1a_32(std::string_view str, std::uint32_t hash = 0x811c9dc5 /* FNV basis 2166136261 */)
{
    for (auto& c : str) {
        hash ^= static_cast<std::uint32_t>(c);
        hash *= 0x01000193; // FNV prime (16777619)
    }

    return hash;
}

[[nodiscard]] constexpr std::int32_t proton(const char* data, std::size_t length = 0)
{
    std::int32_t hash{ 0x55555555 };
    if (!data) {
        return hash;
    }


    if (length > 0) {
        while (length--) {
            hash = (hash >> 27) + (hash << 5) + *reinterpret_cast<const std::uint8_t*>(data++);
        }
    }
    else {
        while (*data) {
            hash = (hash >> 27) + (hash << 5) + *reinterpret_cast<const std::uint8_t*>(data++);
        }
    }

    return hash;
}
}

[[nodiscard]] constexpr std::uint32_t operator "" _fnv1a_32(const char* str, const std::size_t len)
{
    return hash::fnv1a_32(std::string_view{ str, len });
}

[[nodiscard]] constexpr std::int32_t operator "" _proton(const char* str, const std::size_t len)
{
    return hash::proton(str, static_cast<std::uint32_t>(len));
}
