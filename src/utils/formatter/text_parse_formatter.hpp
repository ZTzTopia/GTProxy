#pragma once
#include <fmt/format.h>

#include "../text_parse.hpp"

template<>
struct fmt::formatter<utils::TextParse> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const utils::TextParse& text_parse, FormatContext& ctx) const
    {
        auto out = ctx.out();
        const auto& key_values{ text_parse.get_key_values() };
        for (std::size_t i{ 0 }; i < key_values.size(); ++i) {
            if (i > 0) {
                out = fmt::format_to(out, "\n");
            }

            out = fmt::format_to(out, "{}: {}", key_values[i].first, key_values[i].second);
        }

        return out;
    }
};
