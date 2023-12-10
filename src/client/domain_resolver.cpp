#include <format>
#include <httplib.h>
#include <nlohmann/json.hpp>

#include "domain_resolver.hpp"
#include "../utils/network.hpp"

namespace client::domain_resolver {
Result resolve_domain_name(const std::string& domain_name) 
{
    static httplib::Client cli{ "https://dns.google" };

    httplib::Result res{ cli.Get(std::format("/resolve?name={}&type=A", domain_name)) };
    if (!network::validate_server_response(res)) {
        return { DomainResolverStatus::ServerFail, {} };
    }

    if (res->body.empty()) {
        return { DomainResolverStatus::ServerFail, {} };
    }

    auto j{ nlohmann::json::parse(res->body) };
    const DomainResolverStatus status{ j["Status"] };

    if (status != DomainResolverStatus::NoError) {
        return { status, {} };
    }

    return {
        status,
        j["Answer"][j["Answer"].size() - 1]["data"].get<std::string>()
    };
}
}
