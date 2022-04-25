#pragma once
#include <stdint.h>
#include <string_view>

namespace utils {
    constexpr uint32_t quick_hash(const std::string_view& data) {
        uint32_t hash = 5381;
        for(const auto& c : data)
            hash = ((hash << 5) + hash) + (unsigned char)c;
        return hash;
    }
    constexpr uint32_t operator "" _qh(const char* str, size_t len) {
        return quick_hash(std::string_view(str, len));
    }
}