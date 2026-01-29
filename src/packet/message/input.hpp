#pragma once
#include "../packet_helper.hpp"

namespace packet::message {
struct Input : TextPacket<PacketId::Input, NET_MESSAGE_GENERIC_TEXT> {
    std::string text;

    bool read(const Payload& payload) override
    {
        const auto* text_payload = get_payload_if<TextPayload>(payload);
        if (!text_payload) return false;
        
        text = text_payload->data.get("text", 0);
        return true;
    }

    Payload write() override
    {
        TextParse text_parse{};
        text_parse.add("action", "input");
        // text_parse.add("text", { text });
        // Since the Growtopia client send by `|text|a` without a key for text,
        // we will follow the same format here.
        text_parse.add("", "text", text);
        return TextPayload{ MESSAGE_TYPE, std::move(text_parse) };
    }
};
}
