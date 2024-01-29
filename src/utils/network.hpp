#pragma once
#include <algorithm>
#include <vector>

#include "text_parse.hpp"

namespace network {
enum class HostType {
    IpAddress,
    Hostname
};

inline bool is_valid_ip_address(const std::string& address)
{
    std::vector parts{ TextParse::tokenize(address, ".") };
    if (parts.size() != 4) {
        return false;
    }

    return std::ranges::all_of(parts, [](const std::string& part){
        try {
            return std::stoi(part) >= 0 && std::stoi(part) <= 255;
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
