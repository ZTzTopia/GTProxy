#pragma once

#include <fstream>
#include <string>
#include <string_view>

namespace utils::hash {
[[nodiscard]] constexpr uint32_t fnv1a_32(std::string_view str, uint32_t hash = 0x811c9dc5)
{
    for (auto& c : str) {
        hash ^= static_cast<uint32_t>(c);
        hash *= 0x01000193;
    }

    return hash;
}

[[nodiscard]] constexpr uint32_t proton(const char* data, std::size_t length = 0)
{
    uint32_t hash{ 0x55555555 };
    if (!data) {
        return hash;
    }

    if (length <= 0) {
        length = std::strlen(data);
    }

    if (length > 0) {
        while (length--) {
            hash = (hash >> 27) + (hash << 5) + *reinterpret_cast<const uint8_t*>(data++);
        }
    }

    return hash;
}

[[nodiscard]] inline uint32_t proton_file(const std::string& file_path)
{
    std::ifstream in{ file_path, std::ios::binary };
    if (!in) {
        return 0;
    }

    const std::streampos begin{ in.tellg() };
    in.seekg(0, std::ios::end);
    const std::streampos end{ in.tellg() };
    in.seekg(0, std::ios::beg);

    const std::size_t length{ static_cast<std::size_t>(end - begin) };
    spdlog::debug("Calculating proton hash for file '{}' ({} bytes)", file_path, length);

    std::vector<char> buffer(length);
    in.read(buffer.data(), static_cast<std::streamsize>(length));

    return proton(buffer.data(), length);
}
}

[[nodiscard]] constexpr uint32_t operator "" _fnv1a_32(const char* str, const std::size_t len)
{
    return utils::hash::fnv1a_32(std::string_view{ str, len });
}

[[nodiscard]] constexpr int32_t operator "" _proton(const char* str, const std::size_t len)
{
    return utils::hash::proton(str, static_cast<uint32_t>(len));
}
