#pragma once
#include <vector>
#include <string>
#include <span>
#include <cstring>

template <typename LengthType = std::uint16_t>
class ByteStream {
public:
    ByteStream()
        : read_offset_{ 0 }
    { }

    ByteStream(const std::byte* data, const std::size_t length)
        : data_span_{ data, length }
        , read_offset_{ 0 }
    { }

    ByteStream(std::span<const std::byte> span)
        : data_span_{ span }
        , read_offset_{ 0 }
    { }

    void write_data(const void* ptr, const std::size_t size)
    {
        const auto begin{ static_cast<const std::byte*>(ptr) };
        const std::byte* end{ begin + size };
        data_vec_.insert(data_vec_.end(), begin, end);
    }

    void write_vector(const std::vector<std::byte>& vec, const bool write_length_info = true)
    {
        if (write_length_info) {
            write(static_cast<LengthType>(vec.size()));
        }

        write_data(vec.data(), vec.size());
    }

    template <typename T>
    void write(const T& value)
    {
        write_data(&value, sizeof(T));
    }

    void write(const char* c_str, const bool write_length_info = true)
    {
        write(std::string{ c_str }, write_length_info);
    }

    void write(const std::string& str, const bool write_length_info = true)
    {
        if (write_length_info) {
            write(static_cast<LengthType>(str.size()));
        }

        write_data(str.c_str(), str.size());
    }

    template <typename T>
    ByteStream& operator<<(const T& value)
    {
        write(value);
        return *this;
    }

    bool read_data(void* ptr, const std::size_t size)
    {
        if (get_size() - read_offset_ < size) {
            return false;
        }

        std::memcpy(ptr, get_raw_ptr() + read_offset_, size);
        read_offset_ += size;
        return true;
    }

    bool read_vector(std::vector<std::byte>& vec, LengthType length = 0)
    {
        if (length == 0) {
            if (!read<LengthType>(length)) {
                return false;
            }
        }

        if (get_size() - read_offset_ < static_cast<std::size_t>(length)) {
            return false;
        }

        vec.resize(length);
        return read_data(vec.data(), length);
    }

    template <typename T>
    bool read(T& value)
    {
        return read_data(&value, sizeof(T));
    }

    bool read(std::string& str, LengthType length = 0)
    {
        if (length == 0) {
            if (!read<LengthType>(length)) {
                return false;
            }
        }

        if (get_size() - read_offset_ < static_cast<std::size_t>(length)) {
            return false;
        }

        str.resize(static_cast<std::size_t>(length));
        return read_data(str.data(), static_cast<std::size_t>(length));
    }

    template <typename T>
    ByteStream& operator>>(T& value)
    {
        read(value);
        return *this;
    }

    void backtrack(const std::size_t size) { read_offset_ -= size; }
    void skip(const std::size_t size) { read_offset_ += size; }
    [[nodiscard]] std::size_t get_read_offset() const { return read_offset_; }
    
    [[nodiscard]] std::size_t get_size() const
    {
        return data_vec_.empty() ? data_span_.size() : data_vec_.size(); 
    }

    [[nodiscard]] const std::byte* get_raw_ptr() const
    {
        return data_vec_.empty() ? data_span_.data() : data_vec_.data();
    }

    [[nodiscard]] std::vector<std::byte> get_data() const
    {
        if (!data_vec_.empty()) {
            return data_vec_;
        }

        return { data_span_.begin(), data_span_.end() };
    }

private:
    std::vector<std::byte> data_vec_;
    std::span<const std::byte> data_span_;
    std::size_t read_offset_;
};