#include "resolver.h"

#include <fmt/core.h>

#include <nlohmann/json.hpp>
#include <httplib.h>

using namespace Resolver;

Result Resolver::ResolveHostname(std::string hostname) {
    static httplib::Client cli {"https://dns.google"};

    httplib::Result res = cli.Get(fmt::format("/resolve?name={}&type=A", hostname));
    Result ret{};

    if (!res->body.empty()) {
        auto j = nlohmann::json::parse(res->body);

        ret.Status = j["Status"];

        if (ret.Status != Status::NoError) {
            ret.Ip = "";
            return ret;
        }

        ret.Ip = j["Answer"].at( j["Answer"].size() - 1 )["data"];
    }
    return ret;
}
