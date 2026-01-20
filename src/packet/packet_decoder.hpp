#pragma once
#include <optional>
#include <span>

#include "packet_registry.hpp"
#include "packet_types.hpp"
#include "payload.hpp"
#include "../utils/byte_stream.hpp"
#include "../utils/formatter/packet_variant_formatter.hpp"
#include "../utils/formatter/text_parse_formatter.hpp"

namespace packet {
class PacketDecoder {
public:
    std::optional<std::shared_ptr<IPacket>> decode(std::span<const std::byte> data) const
    {
        auto pkt_log{ spdlog::get("packet") };
        pkt_log->debug(
            "Decoding packet data ({} bytes):{}",
            data.size(),
            spdlog::to_hex(data.begin(), data.end())
        );

        ByteStream stream{ data };

        NetMessageType msg_type{};
        if (!stream.read(msg_type)) {
            return std::nullopt;
        }

        switch (msg_type) {
        case NET_MESSAGE_SERVER_HELLO: {
            TextPayload text_payload{ NET_MESSAGE_SERVER_HELLO };
            Payload payload = text_payload;

            auto packet = PacketRegistry::instance().create(payload);
            return packet;
        }
        case NET_MESSAGE_GENERIC_TEXT:
        case NET_MESSAGE_GAME_MESSAGE: {
            std::string message{};
            stream.read(message, static_cast<uint16_t>(stream.get_size() - sizeof(NetMessageType) - 1));

            TextParse parser{ message };
            spdlog::info("Packet decoded to message:\n{}", fmt::format("{}", parser));

            TextPayload text_payload{ msg_type, std::move(parser) };
            Payload payload = text_payload;
            
            auto packet = PacketRegistry::instance().create(payload);
            if (!packet) {
                // spdlog::debug("No packet structure registered for this message");
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

            if (game_pkt.type == PACKET_CALL_FUNCTION) {
                PacketVariant variant{};
                if (!variant.deserialize(extra)) {
                    // spdlog::warn("Failed to deserialize variant data");
                    return std::nullopt;
                }

                spdlog::info("Packet decoded to variant:\n{}", fmt::format("{}", variant));

                VariantPayload var_payload{ game_pkt, std::move(variant) };
                Payload payload = var_payload;
                
                auto packet = PacketRegistry::instance().create(payload);
                if (!packet) {
                    // spdlog::debug("No packet structure registered for variant: {}", var_payload.function_name());
                    return std::nullopt;
                }
                
                return packet;
            }

            GamePayload game_payload{ game_pkt, std::move(extra) };
            Payload payload = game_payload;
            
            auto packet = PacketRegistry::instance().create(payload);
            if (!packet) {
                // spdlog::debug("No packet structure registered for type {}", static_cast<uint16_t>(game_pkt.type));
                return std::nullopt;
            }

            return packet;
        }
        default:
            // spdlog::warn("Unknown message type: {}", static_cast<uint32_t>(msg_type));
            return std::nullopt;
        }
    }
};
}
