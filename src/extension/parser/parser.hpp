#pragma once
#include "../extension.hpp"
#include "../../packet/packet_types.hpp"
#include "../../packet/packet_variant.hpp"
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

    struct PacketParser : Parser {
        packet::GameUpdatePacket packet;
        std::vector<std::byte> ext_data;
        packet::Variant variant;
    };

    using MessageCallback = eventpp::CallbackList<void(const MessageParser&)>;
    using PacketCallback = eventpp::CallbackList<void(const PacketParser&)>;

    [[nodiscard]] virtual MessageCallback& get_message_callback() = 0;
    [[nodiscard]] virtual PacketCallback& get_packet_callback() = 0;
};
