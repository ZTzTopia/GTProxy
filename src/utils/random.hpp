#pragma once
#include <pcg_random.hpp>

#include "randutils.hpp"

namespace randutils {
using pcg32_rng = random_generator<pcg32>;
}

namespace random {
inline randutils::pcg32_rng static_generator()
{
    static randutils::pcg32_rng pcg_rng{};
    return pcg_rng;
}

inline randutils::pcg32_rng thread_local_generator()
{
    thread_local randutils::pcg32_rng pcg_rng{};
    return pcg_rng;
}

inline randutils::pcg32_rng local_generator()
{
    const randutils::pcg32_rng pcg_rng{};
    return pcg_rng;
}
}