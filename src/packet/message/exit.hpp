#pragma once
#include "../packet_types.hpp"
#include "../packet_helper.hpp"

namespace packet::message {
struct Quit : NetMessage<Quit, NetMessageType::NET_MESSAGE_GAME_MESSAGE> {
    bool read(const TextParse& text_parse) override
    {
        return true;
    }

    void write(ByteStream<>& byte_stream) override
    {
        TextParse text_parse{};
        text_parse.add("action", { "quit" });
        byte_stream.write(text_parse.get_raw(), false);
    }
};

struct QuitToExit : NetMessage<QuitToExit, NetMessageType::NET_MESSAGE_GAME_MESSAGE> {
    bool read(const TextParse& text_parse) override
    {
        return true;
    }

    void write(ByteStream<>& byte_stream) override
    {
        TextParse text_parse{};
        text_parse.add("action", { "quit_to_exit" });
        byte_stream.write(text_parse.get_raw(), false);
    }
};

struct JoinRequest : NetMessage<JoinRequest, NetMessageType::NET_MESSAGE_GAME_MESSAGE> {
    std::string world_name;
    bool invited_world;

    bool read(const TextParse& text_parse) override
    {
        world_name = text_parse.get("name", 1);
        invited_world = text_parse.get("invitedWorld", 1) == "1";
        return true;
    }

    void write(ByteStream<>& byte_stream) override
    {
        TextParse text_parse{};
        text_parse.add("action", { "join_request" });
        text_parse.add("name", { world_name });
        text_parse.add("invitedWorld", { "0" });
        byte_stream.write(text_parse.get_raw(), false);
    }
};

struct ValidateWorld : NetMessage<ValidateWorld, NetMessageType::NET_MESSAGE_GAME_MESSAGE> {
    std::string world_name;

    bool read(const TextParse& text_parse) override
    {
        world_name = text_parse.get("name", 1);
        return true;
    }

    void write(ByteStream<>& byte_stream) override
    {
        TextParse text_parse{};
        text_parse.add("action", { "validate_world" });
        text_parse.add("name", { world_name });
        byte_stream.write(text_parse.get_raw(), false);
    }
};
}
