#pragma once
#include <string_view>

namespace hash {
namespace detail {
    [[nodiscard]] constexpr std::uint32_t rotate_left(std::uint32_t value, int shift)
    {
        return (value << shift) | (value >> (32 - shift));
    }
}

[[nodiscard]] std::uint32_t murmur3_32(std::string_view key, std::uint32_t seed = 0)
{
    const std::uint32_t c1{ 0xcc9e2d51 };
    const std::uint32_t c2{ 0x1b873593 };
    const std::uint32_t r1{ 15 };
    const std::uint32_t r2{ 13 };
    const std::uint32_t m{ 5 };
    const std::uint32_t n{ 0xe6546b64 };

    std::uint32_t hash{ seed };
    std::size_t length{ key.length() };

    const std::uint32_t* data{ reinterpret_cast<const std::uint32_t*>(key.data()) };
    std::size_t nblocks{ length / 4 };

    for (std::size_t i{ 0 }; i < nblocks; ++i) {
        std::uint32_t k{ detail::rotate_left(data[i] * c1, r1) * c2 };
        hash ^= k;
        hash = detail::rotate_left(hash, r2) * m + n;
    }

    const std::uint8_t* tail{ reinterpret_cast<const std::uint8_t*>(data + nblocks) };
    std::uint32_t k{ 0 };

    switch (length & 3) {
    case 3:
        k ^= tail[2] << 16;
    case 2:
        k ^= tail[1] << 8;
    case 1:
        k ^= tail[0];
        k *= c1;
        k = detail::rotate_left(k, r1) * c2;
        hash ^= k;
    }

    hash ^= length;
    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;
    return hash;
}

[[nodiscard]] constexpr std::uint32_t fnv1a_32(std::string_view str, std::uint32_t hash = 0x811c9dc5 /* FNV basis 2166136261 */)
{
    for (char c : str) {
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

[[nodiscard]] std::uint32_t operator "" _mm3_32(const char* str, std::size_t len)
{
    return hash::murmur3_32(std::string_view{ str, len });
}

[[nodiscard]] constexpr std::uint32_t operator "" _fnv1a_32(const char* str, std::size_t len)
{
    return hash::fnv1a_32(std::string_view{ str, len });
}

[[nodiscard]] constexpr std::int32_t operator "" _proton(const char* str, std::size_t len)
{
    return hash::proton(str, static_cast<std::uint32_t>(len));
}
