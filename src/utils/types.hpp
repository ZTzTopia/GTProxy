#pragma once

namespace utils::types {
struct Immobile {
    Immobile() = default;

    Immobile(const Immobile&) = delete;
    Immobile& operator=(const Immobile&) = delete;

    Immobile(Immobile&&) = delete;
    Immobile& operator=(Immobile&&) = delete;
};
}
