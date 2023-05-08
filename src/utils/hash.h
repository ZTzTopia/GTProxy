#pragma once
#include <string_view>

namespace utils {
namespace hash {
constexpr std::size_t fnv1a(const std::string_view& data)
{
    // Fowler/Noll/Vo 1a variant.
    std::size_t prime{ 16777619U };
    std::size_t offset_basis{ 2166136261U };

    if constexpr (sizeof(std::size_t) == 8) {
        // 64-bit
        prime = 1099511628211ULL;
        offset_basis = 14695981039346656037ULL;
    }

    std::size_t hash{ offset_basis };
    for (auto& c : data) {
        hash ^= c;
        hash *= prime;
    }

    return hash;
}

constexpr std::int32_t proton(const char* data, std::size_t length = 0)
{
    std::int32_t hash{ 0x55555555 };
    if (data) {
        if (length > 0) {
            while (length--) {
                hash = (hash >> 27) + (hash << 5) + *reinterpret_cast<const std::uint8_t*>(data++);
            }

            return hash;
        }

        while (*data) {
            hash = (hash >> 27) + (hash << 5) + *reinterpret_cast<const std::uint8_t*>(data++);
        }
    }

    return hash;
}
}
}

constexpr std::size_t operator "" _fh(const char* str, std::size_t len)
{
    return utils::hash::fnv1a(std::string_view{ str, len });
}

constexpr std::uint32_t operator "" _ph(const char* str, std::size_t len)
{
    return utils::hash::proton(str, static_cast<std::uint32_t>(len));
}
