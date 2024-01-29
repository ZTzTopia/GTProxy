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

    struct ParserCallback {
        ParseType type;
        player::Player from;
        player::Player to;
        TextParse text;
    };

    using MessageCallback = eventpp::CallbackList<void(const ParserCallback&)>;

    [[nodiscard]] virtual MessageCallback& get_message_callback() = 0;
};
