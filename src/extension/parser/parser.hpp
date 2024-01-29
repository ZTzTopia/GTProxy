#pragma once
#include "../extension.hpp"
#include "../../player/player.hpp"
#include "../../utils/text_parse.hpp"

struct IParserExtension : extension::IExtension {
    PROVIDE_EXT_UID(0x4ea75473);

    enum class ParseType {
        FromClient,
        FromServer
    };

    struct Parser {
        ParseType type;
        player::Player from;
        player::Player to;
    };

    struct MessageParser : Parser {
        TextParse text;
    };

    using MessageCallback = eventpp::CallbackList<void(const MessageParser&)>;

    [[nodiscard]] virtual MessageCallback& get_message_callback() = 0;
};
