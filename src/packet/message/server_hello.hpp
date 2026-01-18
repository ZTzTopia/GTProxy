#pragma once
#include "../packet_helper.hpp"

namespace packet::message {
struct ServerHello : TextPacket<PacketId::ServerHello, NET_MESSAGE_SERVER_HELLO> {
    bool read(const Payload& payload) override
    {
        return is_payload<TextPayload>(payload);
    }
    
    Payload write() const override
    {
        return TextPayload{ MESSAGE_TYPE };
    }
};
}
