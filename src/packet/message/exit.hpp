#pragma once
#include "../packet_helper.hpp"

namespace packet::message {
struct Quit : TextPacket<PacketId::Quit> {
    bool read(const Payload& payload) override
    {
        return is_payload<TextPayload>(payload);
    }

    Payload write() override
    {
        TextParse text_parse{};
        text_parse.add("action", "quit");
        return TextPayload{ MESSAGE_TYPE, std::move(text_parse) };
    }
};

struct QuitToExit : TextPacket<PacketId::QuitToExit> {
    bool read(const Payload& payload) override
    {
        return is_payload<TextPayload>(payload);
    }

    Payload write() override
    {
        TextParse text_parse{};
        text_parse.add("action", "quit_to_exit");
        return TextPayload{ MESSAGE_TYPE, std::move(text_parse) };
    }
};

struct JoinRequest : TextPacket<PacketId::JoinRequest> {
    std::string world_name;
    bool invited_world;

    bool read(const Payload& payload) override
    {
        const auto* text = get_payload_if<TextPayload>(payload);
        if (!text) return false;
        
        world_name = text->data.get("name", 0);
        invited_world = text->data.get("invitedWorld", 0) == "1";
        return true;
    }

    Payload write() override
    {
        TextParse text_parse{};
        text_parse.add("action", "join_request");
        text_parse.add("name", world_name);
        text_parse.add("invitedWorld", invited_world ? "1" : "0");
        return TextPayload{ MESSAGE_TYPE, std::move(text_parse) };
    }
};

struct ValidateWorld : TextPacket<PacketId::ValidateWorld> {
    std::string world_name;

    bool read(const Payload& payload) override
    {
        const auto* text = get_payload_if<TextPayload>(payload);
        if (!text) return false;
        
        world_name = text->data.get("name", 1);
        return true;
    }

    Payload write() override
    {
        TextParse text_parse{};
        text_parse.add("action", "validate_world");
        text_parse.add("name", world_name);
        return TextPayload{ MESSAGE_TYPE, std::move(text_parse) };
    }
};
}
