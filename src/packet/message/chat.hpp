#pragma once
#include "../packet_helper.hpp"

namespace packet::message {
struct Log : TextPacket<PacketId::Log> {
    std::string msg;

    bool read(const Payload& payload) override
    {
        const auto* text = get_payload_if<TextPayload>(payload);
        if (!text) return false;
        
        msg = text->data.get("msg", 1);
        return true;
    }

    Payload write() const override
    {
        TextParse text_parse{};
        text_parse.add("action", { "log" });
        text_parse.add("msg", { msg });
        return TextPayload{ MESSAGE_TYPE, std::move(text_parse) };
    }
};
}
