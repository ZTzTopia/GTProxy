#include <gtest/gtest.h>
#include "utils/text_parse.hpp"

TEST(TextParseTest, ParseStringWithDefaultDelimiter)
{
    const TextParse tp{ "key1|value1|value2\nkey2|value3" };

    EXPECT_EQ(tp.get("key1", 0), "value1");
    EXPECT_EQ(tp.get("key1", 1), "value2");
    EXPECT_EQ(tp.get("key2", 0), "value3");
    EXPECT_EQ(tp.get("nonexistent", 0), "");
}

TEST(TextParseTest, ParseStringWithCustomDelimiter)
{
    const TextParse tp{ "key1,value1,value2\nkey2,value3", "," };

    EXPECT_EQ(tp.get("key1", 0), "value1");
    EXPECT_EQ(tp.get("key1", 1), "value2");
    EXPECT_EQ(tp.get("key2", 0), "value3");
}

TEST(TextParseTest, GetTypedValues)
{
    const TextParse tp{ "int|123\nfloat|123.456\nstring|hello" };

    EXPECT_EQ(tp.get<int>("int", 0), 123);
    EXPECT_NEAR(tp.get<float>("float", 0), 123.456f, 0.001f);
    EXPECT_EQ(tp.get("string", 0), "hello");
    EXPECT_EQ(tp.get<int>("nonexistent", 0), 0);
}

TEST(TextParseTest, AddSetRemove)
{
    TextParse tp{};

    tp.add("key1", {"value1"});
    EXPECT_EQ(tp.get("key1", 0), "value1");

    tp.set("key1", {"new_value"});
    EXPECT_EQ(tp.get("key1", 0), "new_value");

    tp.remove("key1");
    EXPECT_TRUE(tp.get("key1", 0).empty());
}

TEST(TextParseTest, GetRaw)
{
    TextParse tp{};
    tp.add("key1", {"value1", "value2"});
    tp.add("key2", {"value3"});

    // Note: The order of keys in unordered_map is not guaranteed, so checking exact string might be flaky if we don't account for that.
    // However, for simple cases or if we check containment, it might be fine. 
    // Let's check if the output contains the expected lines.
    std::string raw{ tp.get_raw() };
    EXPECT_NE(raw.find("key1|value1|value2"), std::string::npos);
    EXPECT_NE(raw.find("key2|value3"), std::string::npos);
}

TEST(TextParseTest, GetKeyValues) {
    TextParse tp{};
    tp.add("key1", {"v1", "v2"});

    auto key_values{ tp.get_key_values() };
    ASSERT_FALSE(key_values.empty());

    // Again, order depends on map iteration
    bool found{ false };
    for (const auto& [k, v] : key_values) {
        if (k != "key1") {
            continue;
        }

        EXPECT_EQ(v, "v1|v2");
        found = true;
    }

    EXPECT_TRUE(found);
}

TEST(TextParseTest, IdiotGrowtopiaParse)
{
    const TextParse tp{ "213.179.209.175||-1" };

    EXPECT_EQ(tp.get("213.179.209.175", 0), "");
    EXPECT_EQ(tp.get("213.179.209.175", 1), "-1");
}

TEST(TextParseTest, IdiotGrowtopiaParse2)
{
    const TextParse tp{ "action|input\n|text|/exit" };

    EXPECT_EQ(tp.get("action", 0), "input");
    EXPECT_EQ(tp.get("text", 0), "/exit");
}
