#pragma once
#include "../packet_helper.hpp"

namespace packet::message {
struct Quit : TextPacket<PacketId::Quit> {
    bool read(const Payload& payload) override
    {
        const auto text{ get_payload_if<TextPayload>(payload) };
        if (!text) {
            return false;
        }

        text_parse = text->data;
        return true;
    }

    Payload write() override
    {
        TextParse parse{};
        parse.add("action", "quit");
        return TextPayload{ MESSAGE_TYPE, parse };
    }
};

struct QuitToExit : TextPacket<PacketId::QuitToExit> {
    bool read(const Payload& payload) override
    {
        const auto text{ get_payload_if<TextPayload>(payload) };
        if (!text) {
            return false;
        }

        text_parse = text->data;
        return true;
    }

    Payload write() override
    {
        TextParse parse{};
        parse.add("action", "quit_to_exit");
        return TextPayload{ MESSAGE_TYPE, parse };
    }
};

struct JoinRequest : TextPacket<PacketId::JoinRequest> {
    std::string world_name;
    bool invited_world;

    bool read(const Payload& payload) override
    {
        const auto text{ get_payload_if<TextPayload>(payload) };
        if (!text) {
            return false;
        }

        text_parse = text->data;
        
        world_name = text_parse.get("name", 0);
        invited_world = text_parse.get("invitedWorld", 0) == "1";
        return true;
    }

    Payload write() override
    {
        TextParse parse{};
        parse.add("action", "join_request");
        parse.add("name", world_name);
        parse.add("invitedWorld", invited_world ? "1" : "0");
        return TextPayload{ MESSAGE_TYPE, parse };
    }
};

struct ValidateWorld : TextPacket<PacketId::ValidateWorld> {
    std::string world_name;

    bool read(const Payload& payload) override
    {
        const auto text{ get_payload_if<TextPayload>(payload) };
        if (!text) {
            return false;
        }

        text_parse = text->data;
        
        world_name = text_parse.get("name", 1);
        return true;
    }

    Payload write() override
    {
        TextParse parse{};
        parse.add("action", "validate_world");
        parse.add("name", world_name);
        return TextPayload{ MESSAGE_TYPE, parse };
    }
};
}
