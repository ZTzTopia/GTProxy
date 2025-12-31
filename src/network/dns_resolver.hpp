#pragma once
#include <memory>
#include <string>

namespace network {
enum class DnsStatus {
    NoError,
    FormatError,
    ServerFail,
    NameError,
    NotImplemented,
    Refused,
    YXDomain,
    YXRRSet,
    NXRRSet,
    NotAuth,
    NotZone
};

struct DnsResult {
    DnsStatus status;
    std::string ip;
};

class DnsProvider {
public:
    virtual ~DnsProvider() = default;
    [[nodiscard]] virtual std::string get_host() const = 0;
    [[nodiscard]] virtual std::string get_path() const = 0;
};

class CloudflareDnsProvider final : public DnsProvider {
public:
    [[nodiscard]] std::string get_host() const override { return "cloudflare-dns.com"; }
    [[nodiscard]] std::string get_path() const override { return "/dns-query"; }
};

class GoogleDnsProvider final : public DnsProvider {
public:
    [[nodiscard]] std::string get_host() const override { return "dns.google"; }
    [[nodiscard]] std::string get_path() const override { return "/resolve"; }
};

[[nodiscard]] std::unique_ptr<DnsProvider> create_dns_provider(const std::string& name);

class DnsResolver {
public:
    explicit DnsResolver(std::unique_ptr<DnsProvider> provider);

    DnsResult resolve_domain(const std::string& domain);
    std::string resolve_ip(const std::string& host);

private:
    std::unique_ptr<DnsProvider> provider_;
};
}
