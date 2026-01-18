#include <httplib.h>
#include <fmt/format.h>
#include <glaze/glaze.hpp>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "dns_resolver.hpp"
#include "../utils/network.hpp"

namespace network {
struct DnsAnswer {
    std::string data;
};

struct DnsResponse {
    int status;
    std::vector<DnsAnswer> answer;
};

std::unique_ptr<DnsProvider> create_dns_provider(const std::string& name)
{
    if (name == "cloudflare") {
        return std::make_unique<CloudflareDnsProvider>();
    }

    if (name == "google") {
        return std::make_unique<GoogleDnsProvider>();
    }

    spdlog::warn("Unknown DNS provider: {}, defaulting to Google DNS", name);
    return std::make_unique<GoogleDnsProvider>();
}

DnsResolver::DnsResolver(std::unique_ptr<DnsProvider> provider)
    : provider_{ std::move(provider) }
{

}

DnsResult DnsResolver::resolve_domain(const std::string& domain) const
{
    httplib::Headers headers{
        {"Accept", "application/dns-json"}
    };

    httplib::Client cli{ fmt::format("https://{}", provider_->get_host()) };
    httplib::Result res{ cli.Get(
        fmt::format("{}?name={}&type=A", provider_->get_path(), domain),
        headers
    ) };

    if (!res) {
        spdlog::error(
            "DNS request failed: httplib::Error::{}",
            magic_enum::enum_name(res.error())
        );
        return { DnsStatus::ServerFail, {} };
    }

    if (res.error() != httplib::Error::Success || res->status != 200) {
        spdlog::error(
            "Failed to get DNS response: {}",
            res.error() == httplib::Error::Success
                ? fmt::format("HTTP status: {}", res->status)
                : fmt::format("HTTP error: {}", httplib::to_string(res.error()))
        );
        return { DnsStatus::ServerFail, {} };
    }

    if (res->body.empty()) {
        return { DnsStatus::ServerFail, {} };
    }

    DnsResponse dns_response{};
    if (const auto ec = glz::read<glz::opts{.error_on_unknown_keys = false }>(dns_response, res->body)) {
        spdlog::error("Failed to parse DNS response: {}", glz::format_error(ec, res->body));
        return { DnsStatus::ServerFail, {} };
    }

    const auto status = static_cast<DnsStatus>(dns_response.status);
    if (status != DnsStatus::NoError) {
        return { status, {} };
    }

    if (dns_response.answer.empty()) {
        return { DnsStatus::NameError, {} };
    }

    return { status, dns_response.answer.back().data };
}

std::string DnsResolver::resolve_ip(const std::string& host) const
{
    if (classify_host(host) != HostType::Hostname) {
        return host;
    }

    auto [status, ip] = resolve_domain(host);

    if (status != DnsStatus::NoError) {
        spdlog::error(
            "DNS resolution failed for {}: {}",
            host,
            magic_enum::enum_name(status)
        );
        return {};
    }

    spdlog::info("Resolved {} to {}", host, ip);
    return ip;
}
}

template<> struct glz::meta<network::DnsResponse> {
    static constexpr std::string_view rename_key(const std::string_view key) {
        if (key == "status") {
            return "Status";
        }

        if (key == "answer") {
            return "Answer";
        }

        // ReSharper disable once CppDFALocalValueEscapesFunction
        return key;
    }
};
