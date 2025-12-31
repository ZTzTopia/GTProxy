#pragma once
#include <fmt/format.h>
#include <glm/glm.hpp>

#include "../../packet/packet_variant.hpp"

template<>
struct fmt::formatter<packet::variant> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const packet::variant& var, FormatContext& ctx) const
    {
        switch (packet::PacketVariant::get_type(var)) {
        case packet::VariantType::FLOAT:
            return fmt::format_to(ctx.out(), "[FLOAT]: {}", std::get<float>(var));
        case packet::VariantType::STRING:
            return fmt::format_to(ctx.out(), "[STRING]: {}", std::get<std::string>(var));
        case packet::VariantType::VEC2: {
            const auto& vec{ std::get<glm::vec2>(var) };
            return fmt::format_to(ctx.out(), "[VEC2]: x: {}, y: {}", vec.x, vec.y);
        }
        case packet::VariantType::VEC3: {
            const auto& vec{ std::get<glm::vec3>(var) };
            return fmt::format_to(ctx.out(), "[VEC3]: x: {}, y: {}, z: {}", vec.x, vec.y, vec.z);
        }
        case packet::VariantType::UNSIGNED:
            return fmt::format_to(ctx.out(), "[UNSIGNED]: {}", std::get<uint32_t>(var));
        case packet::VariantType::SIGNED:
            return fmt::format_to(ctx.out(), "[SIGNED]: {}", std::get<int32_t>(var));
        default:
            return fmt::format_to(ctx.out(), "[UNKNOWN]");
        }
    }
};

template<>
struct fmt::formatter<packet::PacketVariant> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const packet::PacketVariant& variant, FormatContext& ctx) const
    {
        auto out = ctx.out();
        const auto& variants{ variant.get_variants() };
        for (std::size_t i{ 0 }; i < variants.size(); ++i) {
            if (i > 0) {
                out = fmt::format_to(out, "\n");
            }

            out = fmt::format_to(out, "[{}] {}", i, variants[i]);
        }

        return out;
    }
};
