#pragma once
#include <span>

#include "packet_types.hpp"
#include "packet_registry.hpp"
#include "packet_variant.hpp"
#include "../utils/byte_stream.hpp"
#include "../utils/text_parse.hpp"
#include "../utils/formatter/text_parse_formatter.hpp"
#include "../utils/formatter/packet_variant_formatter.hpp"

namespace packet {
class PacketDecoder {
public:
    std::shared_ptr<IPacket> try_decode(const std::span<const std::byte> data) const
    {
        ByteStream stream{ data };

        NetMessageType msg_type{};
        if (!stream.read(msg_type)) {
            return {};
        }

        switch (msg_type) {
        case NET_MESSAGE_GENERIC_TEXT:
        case NET_MESSAGE_GAME_MESSAGE: {
            std::string message{};
            stream.read(message, static_cast<uint16_t>(stream.get_size() - sizeof(NetMessageType) - 1));

            TextParse parser{ message };

            spdlog::info(
                "Packet decoded to message:\n{}",
                fmt::format("{}", parser)
            );

            auto packet{ PacketRegistry::instance().create(message) };
            if (!packet) {
                spdlog::warn("No packet structure registered for this message");
                return {};
            }

            std::ignore = packet->read(parser);
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

            // Fucking stupid packet type ever.
            if (game_pkt.type == PACKET_CALL_FUNCTION) {
                PacketVariant variant{};
                if (!variant.deserialize(extra)) {
                    return {};
                }


                spdlog::info(
                    "Packet decoded to variant:\n{}",
                    fmt::format("{}", variant)
                );

                auto packet{ PacketRegistry::instance().create(variant.get(0)) };
                if (!packet) {
                    spdlog::warn("No packet structure registered for this variant");
                    return {};
                }

                std::ignore = packet->read(variant);
                return packet;
            }

            auto packet{ PacketRegistry::instance().create(game_pkt.type) };

            if (!packet) {
                spdlog::warn("No packet structure registered for type {}", static_cast<uint16_t>(game_pkt.type));
                return {};
            }

            std::ignore = packet->read(game_pkt, extra);
            return packet;
        }
        default:
            return {};
        }
    }
};
}
