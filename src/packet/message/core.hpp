#pragma once
#include "../packet_types.hpp"

namespace packet::message {
struct ServerHello : NetMessage<NetMessageType::NET_MESSAGE_SERVER_HELLO> {

};

struct RequestName : NetMessage<NetMessageType::NET_MESSAGE_GENERIC_TEXT> {
    bool is_guest;
    std::string name;
    std::string password;
    std::string rid;

    RequestName()
        : is_guest{ false }
    {

    }

    bool read(const TextParse& text_parse)
    {
        name = text_parse.get("tankIDName");
        password = text_parse.get("tankIDPass");

        if (password.empty()) {
            is_guest = true;
            name = text_parse.get("requestedName");
        }

        rid = text_parse.get("rid");
        return true;
    }
};
}
