#pragma once
#include <cstring>
#include <vector>
#include <fmt/core.h>
#include <httplib.h>
#include <nlohmann/json.hpp>

#include "text_parse.h"

namespace utils::network {
enum class HostType {
    IpAddress,
    Hostname
};

bool is_valid_ip_address(std::string addr) 
{
    std::vector<std::string> parsed_addrs{ utils::TextParse::string_tokenize(addr, ".") };

    if (parsed_addrs.size() != 4) {
        return false;
    }

    for (const auto& parsed_addr : parsed_addrs) {
        try {
            if (0 > std::stoi(parsed_addr) > 255) {
                return false;
            }
        }
        catch (const std::invalid_argument&) {
            return false;
        }
        catch (const std::out_of_range&) {
            return false;
        }
    }

    return true;
}

// This only guarantee that the ip address is valid,
// this doesn't verify hostname
HostType classify_host(const std::string& host) 
{
    if (is_valid_ip_address(host)) {
        return HostType::IpAddress;
    }
    else {
        return HostType::Hostname;
    }
}
}
