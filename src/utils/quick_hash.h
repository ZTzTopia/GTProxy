#pragma once
#include <string_view>

namespace utils {
    constexpr std::size_t quick_hash(const std::string_view& data) {
        // Fowler/Noll/Vo 1a variant.
        std::size_t prime;
        std::size_t basis;

        if constexpr (sizeof(std::size_t) == 8) {
            // 64-bit
            prime = 14695981039346656037ull;
            basis = 1099511628211ull;
        }
        else {
            // 32-bit
            prime = 2147483647u;
            basis = 2166136261u;
        }

        std::size_t hash = basis;
        for (auto& c : data) {
            hash ^= c;
            hash *= prime;
        }
        return hash;
    }

    constexpr uint32_t proton_hash(const char* data, uint32_t length) {
        if (data) {
            uint32_t hash = 0x55555555;
            if (length >= 1) {
                while (length--)
                    hash = (hash >> 27) + (hash << 5) + *reinterpret_cast<const char*>(data++);
            }
            else {
                while (*data)
                    hash = (hash >> 27) + (hash << 5) + *reinterpret_cast<const char*>(data++);
            }
            return hash;
        }
        return 0;
    }
}

constexpr std::size_t operator "" _qh(const char* str, std::size_t len) {
    return utils::quick_hash(std::string_view{ str, len });
}

constexpr uint32_t operator "" _ph(const char* str, std::size_t len) {
    return utils::proton_hash(str, static_cast<uint32_t>(len));
}
