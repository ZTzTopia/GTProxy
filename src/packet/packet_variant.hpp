#pragma once
#include <string>
#include <variant>
#include <vector>
#include <glm/glm.hpp>

#include "../utils/byte_stream.hpp"

namespace packet {
enum class VariantType : uint8_t {
    UNKNOWN,
    FLOAT,
    STRING,
    VEC2,
    VEC3,
    UNSIGNED,
    SIGNED = 9
};

using variant = std::variant<float, std::string, glm::vec2, glm::vec3, uint32_t, int32_t>;

class Variant {
public:
    Variant()
        : variants_{ std::vector<variant>() }
    {

    }

    template<typename... Args>
    explicit Variant(const Args&... args)
        : variants_{ { args... } }
    {

    }

    [[nodiscard]] static VariantType get_type(const variant& var)
    {
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

    [[nodiscard]] std::vector<std::byte> serialize() const
    {
        const size_t size{ variants_.size() };

        ByteStream<uint32_t> byte_stream{};
        byte_stream.write<uint8_t>(size);

        for (size_t i{ 0 }; i < size; i++) {
            VariantType type{ get_type(variants_[i]) };

            byte_stream.write<uint8_t>(i);
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
                byte_stream.write(std::get<uint32_t>(variants_[i]));
            }
            else if (type == VariantType::SIGNED) {
                byte_stream.write(std::get<int32_t>(variants_[i]));
            }
        }

        return byte_stream.get_data();
    }

    [[nodiscard]] bool deserialize(const std::vector<std::byte>& data)
    {
        ByteStream<uint32_t> byte_stream{ const_cast<std::byte*>(data.data()), data.size() };

        uint8_t size{ 0 };
        byte_stream.read(size);

        for (uint8_t i{ 0 }; i < size; i++) {
            uint8_t index{ 0 };
            byte_stream.read(index);

            uint8_t type{ 0 };
            byte_stream.read(type);

            if (type == static_cast<uint8_t>(VariantType::FLOAT)) {
                float value{ 0.0f };
                byte_stream.read(value);
                variants_.emplace_back(value);
            }
            else if (type == static_cast<uint8_t>(VariantType::STRING)) {
                std::string value{};
                byte_stream.read(value);
                variants_.emplace_back(value);
            }
            else if (type == static_cast<uint8_t>(VariantType::VEC2)) {
                glm::vec2 value{};
                byte_stream.read(value.x);
                byte_stream.read(value.y);
                variants_.emplace_back(value);
            }
            else if (type == static_cast<uint8_t>(VariantType::VEC3)) {
                glm::vec3 value{};
                byte_stream.read(value.x);
                byte_stream.read(value.y);
                byte_stream.read(value.z);
                variants_.emplace_back(value);
            }
            else if (type == static_cast<uint8_t>(VariantType::UNSIGNED)) {
                uint32_t value{ 0 };
                byte_stream.read(value);
                variants_.emplace_back(value);
            }
            else if (type == static_cast<uint8_t>(VariantType::SIGNED)) {
                int32_t value{ 0 };
                byte_stream.read(value);
                variants_.emplace_back(value);
            }
        }

        return true;
    }

    template<typename T = std::string>
    void add(const T& value)
    {
        variants_.emplace_back(value);
    }

    template <typename T = std::string>
    [[nodiscard]] T get(const std::size_t index) const
    {
        if (index > variants_.size()) {
            return T{};
        }

        try {
            return std::get<T>(variants_[index]);
        }
        catch (const std::exception&) {
            return T{}; // or some other default value
        }
    }

    void set(const std::size_t index, const variant& value)
    {
        if (index > variants_.size()) {
            return;
        }

        variants_[index] = value;
    }

    [[nodiscard]] std::vector<variant> get_variants() const { return variants_; }
    [[nodiscard]] std::size_t size() const { return variants_.size(); }

private:
    std::vector<variant> variants_;
};
}
