#pragma once

#include <string.h>
#include <vector>
#include <fmt/core.h>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "text_parse.h"

namespace utils {

enum HostType {
    IpAddr,
    Hostname,
    NotValid
};

bool IsValidIp(std::string addr) {
    std::vector<std::string> parsed_addr = utils::TextParse::string_tokenize(addr, ".");

    if (parsed_addr.size() != 4)
        return false;

    for (std::string addr_part : parsed_addr) {
        try {
            if (std::stoi(addr_part) > 255) {
                return false;
            }
        }
        catch (...) {
            return false;
        }
    }
    return true;
}

// this only guarantee that the ip address is valid.
// this doesn't verify hostname.
HostType IsIpOrHostname(std::string addr) {
    if (IsValidIp(addr))
        return HostType::IpAddr;
    else
        return HostType::Hostname;
}

}
