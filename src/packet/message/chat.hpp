#pragma once
#include "../packet_helper.hpp"

namespace packet::message {
struct Log : TextPacket<PacketId::Log> {
    std::string msg;

    bool read(const Payload& payload) override
    {
        const auto text{ get_payload_if<TextPayload>(payload) };
        if (!text) {
            return false;
        };

        text_parse = text->data;
        
        msg = text_parse.get("msg", 1);
        return true;
    }

    Payload write() override
    {
        utils::TextParse parse{};
        parse.add("action", "log");
        parse.add("msg", msg);
        return TextPayload{ MESSAGE_TYPE, parse };
    }
};
}
