#pragma once
#include "../packet_types.hpp"
#include "../packet_helper.hpp"

namespace packet::message {
struct Log : NetMessage<Log, NetMessageType::NET_MESSAGE_GAME_MESSAGE> {
    std::string msg;

    bool read(const TextParse& text_parse) override
    {
        msg = text_parse.get("msg", 1);
        return true;
    }

    void write(ByteStream<>& byte_stream) override
    {
        TextParse text_parse{};
        text_parse.add("action", { "log" });
        text_parse.add("msg", { msg });
        byte_stream.write(text_parse.get_raw(), false);
    }
};
}
