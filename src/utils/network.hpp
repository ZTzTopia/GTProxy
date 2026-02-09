#pragma once
#include <algorithm>
#include <format>
#include <vector>

#include "text_parse.hpp"

namespace utils::network {
enum class HostType {
    IpAddress,
    Hostname
};

inline bool is_valid_ip_address(const std::string& address)
{
    std::vector parts{ utils::TextParse::tokenize(address, ".") };
    if (parts.size() != 4) {
        return false;
    }

    return std::ranges::all_of(parts, [](const std::string_view& part){
        try {
            uint8_t value{};
            if (std::from_chars(part.data(), part.data() + part.size(), value).ec != std::errc{}) {
                return false;
            }

            return value > 0 && value <= 255;
        }
        catch (const std::exception&) {
            return false;
        }
    });
}

constexpr HostType classify_host(const std::string& host)
{
    return is_valid_ip_address(host)
        ? HostType::IpAddress
        : HostType::Hostname;
}

inline std::string format_ip_address(const uint32_t ip_address)
{
    return std::format(
        "{}.{}.{}.{}",
        ip_address & 0xFF,
        (ip_address >> 8) & 0xFF,
        (ip_address >> 16) & 0xFF,
        (ip_address >> 24) & 0xFF
    );
}
}
