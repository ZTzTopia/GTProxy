#pragma once
#include <string>

namespace client::domain_resolver {
enum class DomainResolverStatus {
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

struct Result {
    DomainResolverStatus status_;
    std::string ip_;
};

Result resolve_domain_name(const std::string& domain_name);
}
