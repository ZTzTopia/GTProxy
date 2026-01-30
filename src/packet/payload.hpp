#pragma once
#include <utility>
#include <variant>
#include <vector>

#include "packet_types.hpp"
#include "packet_variant.hpp"
#include "../utils/text_parse.hpp"

namespace packet {
enum class PayloadType : uint8_t {
    Text,
    Game,
    Variant,
    Raw
};

struct TextPayload {
    NetMessageType message_type;
    TextParse data;
    std::vector<std::byte> raw_data; // Original raw bytes for pass-through

    explicit TextPayload(NetMessageType type = NET_MESSAGE_GAME_MESSAGE)
        : message_type{ type }
    { }

    TextPayload(const NetMessageType type, TextParse parser)
        : message_type{ type }
        , data{ std::move(parser) }
    { }

    TextPayload(const NetMessageType type, TextParse parser, std::vector<std::byte> raw)
        : message_type{ type }
        , data{ std::move(parser) }
        , raw_data{ std::move(raw) }
    { }
};

struct GamePayload {
    GameUpdatePacket packet;
    std::vector<std::byte> extra;
    std::vector<std::byte> raw_data;

    GamePayload()
        : packet{}
    { }

    GamePayload(const GameUpdatePacket& pkt, const std::vector<std::byte>& ext)
        : packet{ pkt }
        , extra{ ext }
    { }

    GamePayload(const GameUpdatePacket& pkt, std::vector<std::byte> ext, std::vector<std::byte> raw)
        : packet{ pkt }
        , extra{ std::move(ext) }
        , raw_data{ std::move(raw) }
    { }
};

struct VariantPayload {
    GameUpdatePacket game_packet;
    PacketVariant variant;
    std::vector<std::byte> raw_data;

    explicit VariantPayload(PacketVariant var)
        : game_packet{}
        , variant{ std::move(var) }
    {
        // Default to -1 if not specified
        game_packet.net_id = -1;
        game_packet.decompressed_data_size = -1; // Why Growtopia server default it to -1? what is the other name
        // for this field?
    }

    VariantPayload(const GameUpdatePacket& pkt, PacketVariant var)
        : game_packet{ pkt }
        , variant{ std::move(var) }
    { }

    VariantPayload(const GameUpdatePacket& pkt, PacketVariant var, std::vector<std::byte> raw)
        : game_packet{ pkt }
        , variant{ std::move(var) }
        , raw_data{ std::move(raw) }
    { }

    [[nodiscard]] std::string function_name() const
    {
        return variant.get<std::string>(0);
    }
};

struct RawPayload {
    std::vector<std::byte> data;

    RawPayload() = default;
    explicit RawPayload(std::vector<std::byte> raw) : data{ std::move(raw) } { }
};

using Payload = std::variant<TextPayload, GamePayload, VariantPayload, RawPayload>;

[[nodiscard]] inline PayloadType get_payload_type(const Payload& payload)
{
    return static_cast<PayloadType>(payload.index());
}

template<typename T>
[[nodiscard]] bool is_payload(const Payload& payload)
{
    return std::holds_alternative<T>(payload);
}

template<typename T>
[[nodiscard]] const T& get_payload(const Payload& payload)
{
    return std::get<T>(payload);
}

template<typename T>
[[nodiscard]] const T* get_payload_if(const Payload& payload)
{
    return std::get_if<T>(&payload);
}
}
