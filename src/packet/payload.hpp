#pragma once
#include <variant>
#include <vector>

#include "packet_types.hpp"
#include "packet_variant.hpp"
#include "../utils/text_parse.hpp"

namespace packet {
enum class PayloadType : uint8_t {
    Text,
    Game,
    Variant
};

struct TextPayload {
    NetMessageType message_type;
    TextParse data;

    explicit TextPayload(NetMessageType type = NET_MESSAGE_GAME_MESSAGE)
        : message_type{ type }
    { }

    TextPayload(const NetMessageType type, const TextParse& parser)
        : message_type{ type }
        , data{ parser }
    { }
};

struct GamePayload {
    GameUpdatePacket packet;
    std::vector<std::byte> extra;

    GamePayload()
        : packet{}
    { }

    GamePayload(const GameUpdatePacket& pkt, const std::vector<std::byte>& ext)
        : packet{ pkt }
        , extra{ ext }
    { }
};

struct VariantPayload {
    PacketVariant variant;

    explicit VariantPayload(const PacketVariant& var)
        : variant{ var }
    { }

    [[nodiscard]] std::string function_name() const
    {
        return variant.get<std::string>(0);
    }
};

using Payload = std::variant<TextPayload, GamePayload, VariantPayload>;

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
