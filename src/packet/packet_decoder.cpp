#include "packet_decoder.hpp"

#include <magic_enum/magic_enum.hpp>

#include "../utils/formatter/packet_variant_formatter.hpp"
#include "../utils/formatter/text_parse_formatter.hpp"

namespace packet {
[[nodiscard]] std::optional<std::shared_ptr<IPacket>> PacketDecoder::decode(
    std::span<const std::byte> data,
    const core::Config::LogConfig& log_config,
    std::string_view direction
) {
    spdlog::info("[{}] Decoding packet of size {} bytes", direction, data.size());

    utils::ByteStream stream{ data.data(), data.size() };

    NetMessageType msg_type{};
    if (!stream.read(msg_type)) {
        return std::nullopt;
    }

    switch (msg_type) {
    case NET_MESSAGE_SERVER_HELLO: {
        TextPayload text_payload{ 
            NET_MESSAGE_SERVER_HELLO, 
            utils::TextParse{}, 
            data
        };
        Payload payload = text_payload;

        spdlog::info("Received server hello packet");
        return PacketRegistry::instance().create(payload);
    }
    case NET_MESSAGE_GENERIC_TEXT:
    case NET_MESSAGE_GAME_MESSAGE: {
        std::string message{};
        stream.read(message, static_cast<uint16_t>(stream.get_size() - sizeof(NetMessageType) - 1));

        utils::TextParse parser{ message };
        if (log_config.print_message) {
            spdlog::info(
                "{} ({} bytes):\n{}",
                magic_enum::enum_name(msg_type),
                message.size(),
                parser
            );
        }

        TextPayload text_payload{ msg_type, std::move(parser), data };
        auto packet = PacketRegistry::instance().create(text_payload);
        if (!packet) {
            return std::nullopt;
        }

        return packet;
    }
    case NET_MESSAGE_GAME_PACKET: {
        GameUpdatePacket game_pkt{};
        stream.read(game_pkt);

        const size_t extra_size = static_cast<size_t>(
             game_pkt.data_size > 0
                 ? game_pkt.data_size
                 : stream.get_size() - stream.get_read_offset()
        );

        // Create a view of the extra data
        std::span<const std::byte> extra_view{
            data.data() + stream.get_read_offset(),
            stream.get_size() - stream.get_read_offset()
        };

        spdlog::info(
            "{} with type of {}",
            magic_enum::enum_name(msg_type),
            magic_enum::enum_name(game_pkt.type)
        );

        if (log_config.print_game_update_packet) {
            // View of the GameUpdatePacket bytes
            std::span<const std::byte> game_pkt_view{ 
                data.data() + sizeof(NetMessageType), 
                sizeof(GameUpdatePacket) 
            };
            spdlog::info(
                "Game packet: {}",
                spdlog::to_hex(game_pkt_view.begin(), game_pkt_view.end())
            );
        }

        stream.skip(extra_size);

        if (game_pkt.type == PACKET_CALL_FUNCTION) {
            PacketVariant variant{};
            if (!variant.deserialize(extra_view)) {
                return std::nullopt;
            }

            if (log_config.print_variant) {
                spdlog::info("{}", variant);
            }

            VariantPayload var_payload{ 
                game_pkt, 
                std::move(variant), 
                data 
            };
            Payload payload = var_payload;

            auto packet = PacketRegistry::instance().create(payload);
            if (!packet) {
                return std::nullopt;
            }

            return packet;
        }

        if (log_config.print_extra && extra_size > 1) {
            spdlog::info("Extra data: {}", spdlog::to_hex(extra_view.begin(), extra_view.end()));
        }

        GamePayload game_payload{ 
            game_pkt, 
            extra_view, 
            data 
        };
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
}
