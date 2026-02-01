#pragma once
#include <string>
#include <fmt/format.h>
#include "../packet_helper.hpp"
#include "../../utils/text_parse.hpp"

namespace packet::game {
struct OnSpawn : VariantPacket<PacketId::OnSpawn> {
    std::string spawn;
    int32_t net_id;
    int32_t user_id;
    std::string country_code;
    std::string name;
    glm::i32vec2 position;
    glm::i32vec4 collision;
    int32_t invisible;
    int32_t mod_state;
    int32_t supermod_state;
    int32_t online_id;
    std::string type;
    std::string title_icon;

    OnSpawn()
        : net_id{ -1 }
        , user_id{ 0 }
        , position{ 0, 0 }
        , collision{ 0, 0, 0, 0 }
        , invisible{ 0 }
        , mod_state{ 0 }
        , supermod_state{ 0 }
        , online_id{ 0 }
    {

    }

    bool read(const Payload& payload) override
    {
        const auto var{ get_payload_if<VariantPayload>(payload) };
        if (!var) {
            return false;
        }

        variant = var->variant;
        game_packet = var->game_packet;

        if (variant.size() < 2) {
            return false;
        }

        TextParse parser{};
        const std::string data{ variant.get<std::string>(1) };
        parser.parse(data);

        spawn = parser.get("spawn", 0);
        net_id = parser.get<int32_t>("netID", 0);
        user_id = parser.get<int32_t>("userID", 0);
        country_code = parser.get("country", 0);
        name = parser.get("name", 0);
        position = {
            parser.get<int32_t>("posXY", 0),
            parser.get<int32_t>("posXY", 1)
        };
        collision = {
            parser.get<int32_t>("colrect", 0),
            parser.get<int32_t>("colrect", 1),
            parser.get<int32_t>("colrect", 2),
            parser.get<int32_t>("colrect", 3)
        };
        invisible = parser.get<int32_t>("invis", 0);
        mod_state = parser.get<int32_t>("mstate", 0);
        supermod_state = parser.get<int32_t>("smstate", 0);
        online_id = parser.get<int32_t>("onlineID", 0);
        type = parser.get("type", 0);
        title_icon = parser.get("titleIcon", 0);

        spdlog::debug(
            "OnSpawn(spawn={}, net_id={}, user_id={}, country_code={}, name={}, position=({}, {}), "
            "collision=({}, {}, {}, {}), invisible={}, mod_state={}, supermod_state={}, online_id={}, type={}, title_icon={})",
            spawn, net_id, user_id, country_code, name, position.x, position.y,
            collision.x, collision.y, collision.z, collision.w,
            invisible, mod_state, supermod_state, online_id, type, title_icon
        );
        return true;
    }

    Payload write() override
    {
        TextParse parser{};
        parser.add("spawn", spawn);
        parser.add("netID", net_id);
        parser.add("userID", user_id);
        parser.add("name", name);
        parser.add("titleIcon", title_icon);
        parser.add("country", country_code);
        parser.add("posXY", position.x, position.y);
        parser.add("colrect", collision.x, collision.y, collision.z, collision.w);
        parser.add("invis", invisible);
        parser.add("mstate", mod_state);
        parser.add("smstate", supermod_state);
        parser.add("onlineID", "");
        parser.add("type", type);

        spdlog::debug("Raw OnSpawn data: {}", parser.get_raw());
        const PacketVariant write_variant{
            "OnSpawn",
            parser.get_raw()
        };

        return VariantPayload{ game_packet, write_variant };
    }
};

struct OnRemove : VariantPacket<PacketId::OnRemove> {
    int32_t net_id;
    int32_t player_id;

    OnRemove()
        : net_id{ -1 }
        , player_id{ 0 }
    {

    }

    bool read(const Payload& payload) override
    {
        const auto var{ get_payload_if<VariantPayload>(payload) };
        if (!var) {
            return false;
        }

        variant = var->variant;
        game_packet = var->game_packet;

        if (variant.size() < 3) {
            return false;
        }

        TextParse parser{};

        const std::string net_id_data{ variant.get<std::string>(1) };
        parser.parse(net_id_data);

        if (parser.contains("netID")) {
            net_id = parser.get<int32_t>("netID", 0);
        }

        const std::string player_id_data{ variant.get<std::string>(2) };
        parser.parse(player_id_data);

        if (parser.contains("pId")) {
            player_id = parser.get<int32_t>("pId", 0);
        }

        return true;
    }

    Payload write() override
    {
        const PacketVariant variant{
            "OnRemove",
            fmt::format("netID|{}", net_id),
            fmt::format("pId|{}", player_id)
        };
        return VariantPayload{ game_packet, variant };
    }
};
}
