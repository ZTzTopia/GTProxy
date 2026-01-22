#pragma once
#include <optional>
#include <span>
#include <string_view>

#include "packet_registry.hpp"
#include "packet_types.hpp"
#include "payload.hpp"
#include "../core/config.hpp"
#include "../utils/byte_stream.hpp"
#include "../utils/formatter/packet_variant_formatter.hpp"
#include "../utils/formatter/text_parse_formatter.hpp"

namespace packet {
class PacketDecoder {
public:
    std::optional<std::shared_ptr<IPacket>> decode(
        std::span<const std::byte> data,
        const core::Config::LogConfig& log_config,
        std::string_view direction
    ) const {
        spdlog::info("[{}] Decoding packet of size {} bytes", direction, data.size());

        auto pkt_log{ spdlog::get("packet") };
        ByteStream stream{ data };

        NetMessageType msg_type{};
        if (!stream.read(msg_type)) {
            return std::nullopt;
        }

        switch (msg_type) {
        case NET_MESSAGE_SERVER_HELLO: {
            TextPayload text_payload{ NET_MESSAGE_SERVER_HELLO };
            Payload payload = text_payload;

            spdlog::info("Received server hello packet");
            return PacketRegistry::instance().create(payload);
        }
        case NET_MESSAGE_GENERIC_TEXT:
        case NET_MESSAGE_GAME_MESSAGE: {
            std::string message{};
            stream.read(message, static_cast<uint16_t>(stream.get_size() - sizeof(NetMessageType) - 1));

            TextParse parser{ message };
            if (log_config.print_message) {
                spdlog::info(
                    "{} ({} bytes):\n{}",
                    magic_enum::enum_name(msg_type),
                    message.size(),
                    parser
                );
            }

            TextPayload text_payload{ msg_type, std::move(parser) };
            auto packet = PacketRegistry::instance().create(text_payload);
            if (!packet) {
                return std::nullopt;
            }

            return packet;
        }
        case NET_MESSAGE_GAME_PACKET: {
            GameUpdatePacket game_pkt{};
            stream.read(game_pkt);

            std::vector<std::byte> extra{};
            stream.read_vector(extra, static_cast<uint16_t>(
                 game_pkt.data_size > 0
                     ? game_pkt.data_size
                     : stream.get_size() - stream.get_read_offset()
            ));

            spdlog::info(
                "{} with type of {}",
                magic_enum::enum_name(msg_type),
                magic_enum::enum_name(game_pkt.type)
            );

            stream.backtrack(extra.size() + sizeof(GameUpdatePacket));

            std::vector<std::byte> game_packet_bytes{};
            stream.read_vector(game_packet_bytes, sizeof(GameUpdatePacket));

            if (log_config.print_game_update_packet) {
                spdlog::info(
                    "Game packet: {}",
                    spdlog::to_hex(game_packet_bytes.begin(), game_packet_bytes.end())
                );
            }

            stream.skip(extra.size());

            if (game_pkt.type == PACKET_CALL_FUNCTION) {
                PacketVariant variant{};
                if (!variant.deserialize(extra)) {
                    return std::nullopt;
                }

                if (log_config.print_variant) {
                    spdlog::info("{}", variant);
                }

                VariantPayload var_payload{ game_pkt, std::move(variant) };
                Payload payload = var_payload;

                auto packet = PacketRegistry::instance().create(payload);
                if (!packet) {
                    return std::nullopt;
                }

                return packet;
            }

            if (log_config.print_extra && extra.size() > 1) {
                spdlog::info("Extra data: {}", spdlog::to_hex(extra.begin(), extra.end()));
            }

            GamePayload game_payload{ game_pkt, std::move(extra) };
            auto packet = PacketRegistry::instance().create(game_payload);
            if (!packet) {
                return std::nullopt;
            }

            return packet;
        }
        default:
            return std::nullopt;
        }
    }
};
}
