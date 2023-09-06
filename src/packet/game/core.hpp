#pragma once
#include "../packet_types.hpp"
#include "../packet_variant.hpp"

namespace packet::game {
struct OnSuperMainStartAcceptLogonHrdxs47254722215a : NetPacket<PacketType::PACKET_CALL_FUNCTION> {
    std::int32_t item_hash;
    std::string cdn_url;
    std::string cdn_path;
    
    void write(GameUpdatePacket& game_update_packet, std::vector<std::byte>& ext_data) override
    {
        game_update_packet.net_id = -1;

        TextParse text_parse{};
        text_parse.add("proto", { "192" });
        text_parse.add("choosemusic", { "audio/mp3/about_theme.mp3" });
        text_parse.add("active_holiday", { "0" });
        text_parse.add("wing_week_day", { "0" });
        text_parse.add("ubi_week_day", { "0" });
        text_parse.add("server_tick", { "0" });
        text_parse.add("clash_active", { "1" });
        text_parse.add("drop_lavacheck_faster", { "1" });
        text_parse.add("isPayingUser", { "0" });
        text_parse.add("usingStoreNavigation", { "1" });
        text_parse.add("enableInventoryTab", { "1" });
        text_parse.add("bigBackpack", { "1" });

        Variant variant{
            "OnSuperMainStartAcceptLogonHrdxs47254722215a",
            item_hash,
            cdn_url,
            cdn_path,
            "useless.lmao",
            text_parse.get_raw()
        };

        ext_data = variant.serialize();
    }
};
}
