#include "domain_resolver.h"

#include <fmt/core.h>
#include <httplib.h>
#include <nlohmann/json.hpp>

namespace domain_resolver {
Result resolve_domain_name(const std::string& domain_name) 
{
    static httplib::Client cli{ "https://dns.google" };

    httplib::Result res{ cli.Get(fmt::format("/resolve?name={}&type=A", domain_name)) };
    if (!res->body.empty()) {
        auto j{ nlohmann::json::parse(res->body) };
        DomainResolverStatus status{ j["Status"] };

        if (status != DomainResolverStatus::NoError) {
            return { "", status };
        }

        return {
            j["Answer"].at(j["Answer"].size() - 1)["data"],
            status
        };
    }

    return {};
}
}
