#pragma once
#include <cstdint>
#include <memory>
#include <string>

class BinaryReader {
public:
    explicit BinaryReader(void* data) : m_data(static_cast<uint8_t*>(data)), m_pos(0) {}
    ~BinaryReader() = default;
    
    template <typename T>
    T read()
    {
        T data;
        std::memcpy(&data, m_data + m_pos, sizeof(T));
        m_pos += sizeof(T);
        return data;
    }

    std::string read_string()
    {
        uint16_t length{ read_u16() };
        std::string string{ reinterpret_cast<char*>(m_data + m_pos), length };
        m_pos += length;
        return string;
    }

    uint8_t read_u8() { return read<uint8_t>(); }
    uint16_t read_u16() { return read<uint16_t>(); }
    uint32_t read_u32() { return read<uint32_t>(); }
    uint64_t read_u64() { return read<uint64_t>(); }
    int8_t read_i8() { return read<int8_t>(); }
    int16_t read_i16() { return read<int16_t>(); }
    int32_t read_i32() { return read<int32_t>(); }
    int64_t read_i64() { return read<int64_t>(); }

    bool read_bool() { return read<bool>(); }
    float read_float() { return read<float>(); }
    double read_double() { return read<double>(); }

    void skip(std::size_t len) { m_pos += len; }
    void skip_string() { m_pos += read_u16(); }
    void back(std::size_t len) { m_pos -= len; }

    [[nodiscard]] std::size_t position() const { return m_pos; }

private:
    uint8_t* m_data;
    std::size_t m_pos;
};
