#pragma once

namespace utils::types {
struct NoCopy {
    NoCopy() = default;

    NoCopy(const NoCopy&) = delete;
    NoCopy& operator=(const NoCopy&) = delete;
};

struct NoMove {
    NoMove() = default;

    NoMove(NoMove&&) = delete;
    NoMove& operator=(NoMove&&) = delete;
};

struct Immobile {
    Immobile() = default;

    Immobile(const Immobile&) = delete;
    Immobile& operator=(const Immobile&) = delete;

    Immobile(Immobile&&) = delete;
    Immobile& operator=(Immobile&&) = delete;
};
}
