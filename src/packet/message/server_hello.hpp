#pragma once
#include "../packet_helper.hpp"

namespace packet::message {
struct ServerHello : TextPacket<PacketId::ServerHello, NET_MESSAGE_SERVER_HELLO> {
    bool read(const Payload& payload) override
    {
        const auto* text{ get_payload_if<TextPayload>(payload) };
        if (!text) {
            return false;
        }

        text_parse = text->data;
        return true;
    }
    
    Payload write() override
    {
        return TextPayload{ MESSAGE_TYPE };
    }
};
}
