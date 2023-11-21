#pragma once
#include <string>
#include <variant>
#include <vector>
#include <glm/glm.hpp>

#include "../utils/byte_stream.hpp"

enum class VariantType : std::uint8_t {
    UNKNOWN,
    FLOAT,
    STRING,
    VEC2,
    VEC3,
    UNSIGNED,
    SIGNED = 9
};

using variant = std::variant<float, std::string, glm::vec2, glm::vec3, std::uint32_t, std::int32_t>;

class Variant {
public:
    template<typename... Args>
    explicit Variant(const Args&... args) 
        : variants_{ { args... } } 
    {

    }

    [[nodiscard]] static VariantType get_type(const variant& var) {
        switch (var.index()) {
        case 0:
            return VariantType::FLOAT;
        case 1:
            return VariantType::STRING;
        case 2:
            return VariantType::VEC2;
        case 3:
            return VariantType::VEC3;
        case 4:
            return VariantType::UNSIGNED;
        case 5:
            return VariantType::SIGNED;
        default:
            return VariantType::UNKNOWN;
        }
    }

    std::vector<std::byte> serialize() const
    {
        const std::uint8_t size{ variants_.size() };

        ByteStream<std::uint32_t> byte_stream{};
        byte_stream.write(size);

        for (std::uint8_t i{ 0 }; i < size; i++) {
            VariantType type{ get_type(variants_[i]) };

            byte_stream.write(i);
            byte_stream.write(static_cast<std::underlying_type_t<VariantType>>(type));

            if (type == VariantType::FLOAT) {
                byte_stream.write(std::get<float>(variants_[i]));
            }
            else if (type == VariantType::STRING) {
                byte_stream.write(std::get<std::string>(variants_[i]));
            }
            else if (type == VariantType::VEC2) {
                glm::vec2 vec{ std::get<glm::vec2>(variants_[i]) };
                byte_stream.write(vec.x);
                byte_stream.write(vec.y);
            }
            else if (type == VariantType::VEC3) {
                glm::vec3 vec{ std::get<glm::vec3>(variants_[i]) };
                byte_stream.write(vec.x);
                byte_stream.write(vec.y);
                byte_stream.write(vec.z);
            }
            else if (type == VariantType::UNSIGNED) {
                byte_stream.write(std::get<std::uint32_t>(variants_[i]));
            }
            else if (type == VariantType::SIGNED) {
                byte_stream.write(std::get<std::int32_t>(variants_[i]));
            }
        }

        return byte_stream.get_data();
    }

    [[nodiscard]] std::vector<variant> variants() const { return variants_; }
    [[nodiscard]] variant get(const std::size_t index) const
    { 
        if (index > variants_.size()) { 
            return {}; 
        } 

        return variants_[index]; 
    }

private:
    std::vector<variant> variants_;
};
