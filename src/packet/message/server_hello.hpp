#pragma once
#include "../packet_helper.hpp"
#include "../packet_types.hpp"
#include "../../utils/byte_stream.hpp"

namespace packet::message {
struct ServerHello : NetMessage<ServerHello, NetMessageType::NET_MESSAGE_SERVER_HELLO> {
    bool read(const TextParse& text_parse) override { return true; }
    void write(ByteStream<>& byte_stream) override { }
};
}
