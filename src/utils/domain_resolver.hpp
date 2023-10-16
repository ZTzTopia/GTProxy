#pragma once

#include <string>

namespace domain_resolver {
enum class DomainResolverStatus : uint8_t {
    NoError,
    FormErr,
    ServerFail,
    NXDomain,
    NotImp,
    Refused,
    YXDomain,
    XReset,
    NotAuth,
    NotZone
};

struct Result {
    std::string ip_;
    DomainResolverStatus status_;
};

Result resolve_domain_name(const std::string& domain_name);
}
