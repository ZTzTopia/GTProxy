#include <gtest/gtest.h>
#include "utils/byte_stream.hpp"

TEST(ByteStreamTest, WriteAndReadBasicTypes)
{
    ByteStream bs{};
    bs.write<int>(42);
    bs.write<float>(3.14f);

    int i{};
    float f{};
    
    bs.read(i);
    bs.read(f);

    EXPECT_EQ(i, 42);
    EXPECT_EQ(f, 3.14f);
}

TEST(ByteStreamTest, WriteAndReadString)
{
    ByteStream bs{};
    const std::string original{ "hello world" };
    bs.write(original);

    std::string result;
    bs.read(result);

    EXPECT_EQ(result, original);
}

TEST(ByteStreamTest, WriteAndReadVector)
{
    ByteStream bs{};
    const std::vector data{ std::byte{ 0x01 }, std::byte{ 0x02 }, std::byte{ 0x03 } };
    bs.write_vector(data);

    std::vector<std::byte> result;
    bs.read_vector(result);

    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], std::byte{ 0x01 });
    EXPECT_EQ(result[2], std::byte{ 0x03 });
}

TEST(ByteStreamTest, ReadVectorWithExplicitLength)
{
    ByteStream bs{};
    const std::vector data{ std::byte{ 0xA }, std::byte{ 0xB } };
    // Write without length prefix
    bs.write_data(data.data(), data.size());

    std::vector<std::byte> result;
    // Read with explicit length
    const bool success{ bs.read_vector(result, 2) };

    EXPECT_TRUE(success);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], std::byte{ 0xA });
}

TEST(ByteStreamTest, StreamOperators)
{
    ByteStream bs{};
    bs << 100 << std::string("test");

    int val{};
    std::string str{};

    bs >> val >> str;

    EXPECT_EQ(val, 100);
    EXPECT_EQ(str, "test");
}

TEST(ByteStreamTest, SpanConstructor)
{
    std::vector raw{
        std::byte{ 0x05 },
        std::byte{ 0x00 },
        std::byte{ 0x01 },
        std::byte{ 0x02 },
        std::byte{ 0x03 },
        std::byte{ 0x04 },
        std::byte{ 0x05 }
    };
    // 0x05, 0x00 is length (5) in little endian uint16_t, followed by 5 bytes

    ByteStream bs{ raw };

    std::vector<std::byte> result{};
    // Default ReadVector reads length first (which is 5 at the beginning) and then payload
    bs.read_vector(result);

    EXPECT_EQ(result.size(), 5);
}
