#pragma once
#include <format>

#include "../packet_types.hpp"

namespace packet::core {
struct ServerHello : NetMessage<NetMessageType::NET_MESSAGE_SERVER_HELLO> {
    void write(ByteStream<std::uint16_t>& byte_stream)
    {

    }
};
}
