#pragma once
#include <string_view>
#include <string>
#include <unordered_map>
#include <vector>
#include <regex>

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
    OnNameChanged,
    OnChangeSkin,
    Padding2 = 0x2000,
    Disconnect,
    TileChangeRequest,
    SendMapData,
    SendTileUpdateData,
    SendItemDatabaseData,
    SendInventoryState,
    ModifyItemInventory,
    ItemChangeObject,
    Padding3 = 0x3000,
    OnSendToServer,
    OnSpawn,
    OnRemove,
    OnSuperMainStartAcceptLogonHrdxs47254722215a,
    Unknown = std::numeric_limits<uint32_t>::max(),
};

struct TextRegexPattern {
    std::string pattern;
    PacketId id;
    std::regex compiled;

    TextRegexPattern(std::string_view pat, PacketId packet_id)
        : pattern{ pat }
        , id{ packet_id }
        , compiled{ pattern, std::regex::optimize }
    { }
};

inline std::vector<TextRegexPattern>& get_text_regex_patterns()
{
    static std::vector<TextRegexPattern> patterns = []() {
        std::vector<TextRegexPattern> p;
        p.emplace_back(R"(^action\|quit$)", PacketId::Quit);
        p.emplace_back(R"(^action\|quit_to_exit$)", PacketId::QuitToExit);
        p.emplace_back(R"(^action\|join_request$)", PacketId::JoinRequest);
        p.emplace_back(R"(^action\|validate_world$)", PacketId::ValidateWorld);
        p.emplace_back(R"(^action\|input)", PacketId::Input);
        p.emplace_back(R"(^action\|log$)", PacketId::Log);
        return p;
    }();
    return patterns;
}

inline const std::unordered_map<std::string_view, PacketId> VARIANT_FUNCTION_MAP = {
    { "OnSendToServer", PacketId::OnSendToServer },
    { "OnSpawn", PacketId::OnSpawn },
    { "OnRemove", PacketId::OnRemove },
    { "OnNameChanged", PacketId::OnNameChanged },
    { "OnChangeSkin", PacketId::OnChangeSkin },
    { "OnSuperMainStartAcceptLogonHrdxs47254722215a", PacketId::OnSuperMainStartAcceptLogonHrdxs47254722215a },
};

inline const std::unordered_map<PacketType, PacketId> GAME_PACKET_MAP = {
    { PACKET_DISCONNECT, PacketId::Disconnect },
    { PACKET_TILE_CHANGE_REQUEST, PacketId::TileChangeRequest },
    { PACKET_SEND_MAP_DATA, PacketId::SendMapData },
    { PACKET_SEND_TILE_UPDATE_DATA, PacketId::SendTileUpdateData },
    { PACKET_SEND_ITEM_DATABASE_DATA, PacketId::SendItemDatabaseData },
    { PACKET_SEND_INVENTORY_STATE, PacketId::SendInventoryState },
    { PACKET_MODIFY_ITEM_INVENTORY, PacketId::ModifyItemInventory },
    { PACKET_ITEM_CHANGE_OBJECT, PacketId::ItemChangeObject },
};

[[nodiscard]] inline PacketId derive_packet_id(const TextPayload& payload)
{
    if (payload.message_type == NET_MESSAGE_SERVER_HELLO) {
        return PacketId::ServerHello;
    }

    const std::string raw{ payload.data.get_raw() };

    for (const auto& entry : get_text_regex_patterns()) {
        if (std::regex_search(raw, entry.compiled)) {
            return entry.id;
        }
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
