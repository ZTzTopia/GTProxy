#pragma once
#include <string_view>

namespace domain_resolver {
enum class DomainResolverStatus : uint8_t {
    NoError, // No Error
    FormatError, // The name server was unable to interpret the query.
    ServerFail, // The name server was unable to process this query due to a problem with the name server.
    NameError, // Meaningful only for responses from an authoritative name server, this code signifies that the domain name referenced in the query does not exist.
    NotImplemented, // The name server does not support the requested kind of query.
    Refused, // The name server refuses to perform the specified operation for policy reasons.
    YXDomain, // Name Exists when it should not.
    YXRRSet, // RR Set Exists when it should not.
    NXRRSet, // RR Set that should exist does not.
    NotAuth, // Server Not Authoritative for zone / Not Authorized.
    NotZone // Name not contained in zone.
};

struct Result {
    DomainResolverStatus status_;
    std::string_view ip_;
};

Result resolve_domain_name(const std::string& domain_name);
}
