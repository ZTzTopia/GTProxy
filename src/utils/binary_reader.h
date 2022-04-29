#pragma once
#include <cstring>
#include <stdint.h>
#include <stdlib.h>
#include <string>

class BinaryReader
{
public:
    BinaryReader(uint8_t* data, size_t len) : m_pos(0) {
        m_data = (uint8_t*)malloc(len);
        std::memset(m_data, 0, len);
        std::memcpy(m_data, data, len);
    }
    ~BinaryReader() {
        free(this->m_data);
    }

    uint8_t read_byte() {
        uint8_t val;
        std::memcpy(&val, this->m_data + this->m_pos, sizeof(uint8_t));
        this->m_pos += sizeof(uint8_t);
        return val;
    }
    bool read_bool() {
        bool val;
        std::memcpy(&val, this->m_data + this->m_pos, sizeof(bool));
        this->m_pos += sizeof(bool);
        return val;
    }
    uint16_t read_ushort() {
        uint16_t val;
        std::memcpy(&val, this->m_data + this->m_pos, sizeof(uint16_t));
        this->m_pos += sizeof(uint16_t);
        return val;
    }
    uint32_t read_uint() {
        uint32_t val;
        std::memcpy(&val, this->m_data + this->m_pos, sizeof(uint32_t));
        this->m_pos += sizeof(uint32_t);
        return val;
    }
    int read_int() {
        int val;
        std::memcpy(&val, this->m_data + this->m_pos, sizeof(int));
        this->m_pos += sizeof(int);
        return val;
    }
    float read_float() {
        float val;
        std::memcpy(&val, this->m_data + this->m_pos, sizeof(float));
        this->m_pos += sizeof(float);
        return val;
    }
    std::string read_string() {
        std::string val;
        uint16_t str_len = *(int16_t*)&this->m_data[this->m_pos];
        val = std::string(reinterpret_cast<char*>(this->m_data + this->m_pos + 2), str_len);
        this->m_pos += sizeof(uint16_t) + str_len;
        return val;
    }
    void copy(void* dest, size_t len) {
        std::memcpy(dest, this->m_data + this->m_pos, len);
        this->m_pos += static_cast<uint32_t>(len);
    }
    void skip(uint32_t len) {
        this->m_pos += len;
    }
    void back(uint32_t len) {
        this->m_pos -= len;
    }
private:
    uint8_t* m_data;
    uint32_t m_pos;
};