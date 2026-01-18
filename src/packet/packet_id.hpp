#pragma once
#include <cstdint>
#include <string_view>
#include <unordered_map>

#include "payload.hpp"

namespace packet {

enum class PacketId : uint32_t {
    ServerHello,
    Padding = 0x1000,
    Quit,
    QuitToExit,
    JoinRequest,
    ValidateWorld,
    Input,
    Log,
    Padding2 = 0x2000,
    Disconnect,
    Padding3 = 0x3000,
    OnSendToServer,
    Unknown = std::numeric_limits<uint32_t>::max(),
};

inline const std::unordered_map<std::string_view, PacketId> TEXT_ACTION_MAP = {
    { "action|quit", PacketId::Quit },
    { "action|quit_to_exit", PacketId::QuitToExit },
    { "action|join_request", PacketId::JoinRequest },
    { "action|validate_world", PacketId::ValidateWorld },
    { "action|input", PacketId::Input },
    { "action|log", PacketId::Log },
};

inline const std::unordered_map<std::string_view, PacketId> VARIANT_FUNCTION_MAP = {
    { "OnSendToServer", PacketId::OnSendToServer },
};

inline const std::unordered_map<PacketType, PacketId> GAME_PACKET_MAP = {
    { PACKET_DISCONNECT, PacketId::Disconnect },
};

[[nodiscard]] inline PacketId derive_packet_id(const TextPayload& payload)
{
    if (payload.message_type == NET_MESSAGE_SERVER_HELLO) {
        return PacketId::ServerHello;
    }

    const std::string raw{ payload.data.get_raw() };
    const auto newline_pos{ raw.find('\n') };
    const std::string_view first_line{
        (newline_pos != std::string::npos)
            ? std::string_view(raw).substr(0, newline_pos)
            : std::string_view(raw)
    };

    if (const auto it{ TEXT_ACTION_MAP.find(first_line) }; it != TEXT_ACTION_MAP.end()) {
        return it->second;
    }

    return PacketId::Unknown;
}

[[nodiscard]] inline PacketId derive_packet_id(const GamePayload& payload)
{
    if (const auto it{ GAME_PACKET_MAP.find(payload.packet.type) }; it != GAME_PACKET_MAP.end()) {
        return it->second;
    }

    return PacketId::Unknown;
}

[[nodiscard]] inline PacketId derive_packet_id(const VariantPayload& payload)
{
    const std::string func_name{ payload.function_name() };
    if (const auto it{ VARIANT_FUNCTION_MAP.find(func_name) }; it != VARIANT_FUNCTION_MAP.end()) {
        return it->second;
    }

    return PacketId::Unknown;
}

[[nodiscard]] inline PacketId derive_packet_id(const Payload& payload)
{
    return std::visit([](const auto& p) { return derive_packet_id(p); }, payload);
}
}
