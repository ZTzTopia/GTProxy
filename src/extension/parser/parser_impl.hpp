#pragma once
#include "parser.hpp"
#include "../../client/client.hpp"
#include "../../core/core.hpp"
#include "../../server/server.hpp"
#include "../../utils/byte_stream.hpp"

namespace extension::parser {
class ParserExtension final : public IParserExtension {
    core::Core* core_;

    MessageCallback message_callback_;
    PacketCallback packet_callback_;

public:
    explicit ParserExtension(core::Core* core)
        : core_{ core }
    {

    }

    ~ParserExtension() override = default;

    void init() override
    {
        core_->get_server()->get_message_callback().prepend([this](
            const player::Player& from, const player::Player& to, const std::string& message
        ) {
            const TextParse text{ message };
            message_callback_(MessageParser{
                ParseType::FromClient,
                from,
                to,
                text
            });
        });

        core_->get_client()->get_message_callback().prepend([this](
            const player::Player& from, const player::Player& to, const std::string& message
        ) {
            const TextParse text{ message };
            message_callback_(MessageParser{
                ParseType::FromServer,
                from,
                to,
                text
            });
        });

        core_->get_client()->get_packet_callback().prepend([this](
            const player::Player& from, const player::Player& to, const std::vector<std::byte>& packet
        ) {
            ByteStream byte_stream{ const_cast<std::byte*>(packet.data()), packet.size() };
            byte_stream.skip(sizeof(packet::NetMessageType));

            packet::GameUpdatePacket game_update_packet{};
            byte_stream.read(game_update_packet);

            std::vector<std::byte> ext_data{};
            byte_stream.read_vector(ext_data, game_update_packet.data_size);

            if (game_update_packet.type != packet::PACKET_CALL_FUNCTION) {
                packet_callback_(PacketParser{
                    ParseType::FromServer,
                    from,
                    to,
                    game_update_packet,
                    ext_data,
                    packet::Variant{}
                });
                return;
            }

            packet::Variant variant{};
            if (!variant.deserialize(ext_data)) {
                spdlog::warn("Failed to deserialize variant");
                return;
            }

            packet_callback_(PacketParser{
                ParseType::FromServer,
                from,
                to,
                game_update_packet,
                ext_data,
                variant
            });
        });

        core_->get_server()->get_packet_callback().prepend([this](
            const player::Player& from, const player::Player& to, const std::vector<std::byte>& packet
        ) {
            ByteStream byte_stream{ const_cast<std::byte*>(packet.data()), packet.size() };
            byte_stream.skip(sizeof(packet::NetMessageType));

            packet::GameUpdatePacket game_update_packet{};
            byte_stream.read(game_update_packet);

            std::vector<std::byte> ext_data{};
            ext_data.resize(game_update_packet.data_size);
            byte_stream.read_data(ext_data.data(), game_update_packet.data_size);

            if (game_update_packet.type != packet::PACKET_CALL_FUNCTION) {
                packet_callback_(PacketParser{
                    ParseType::FromClient,
                    from,
                    to,
                    game_update_packet,
                    ext_data,
                    packet::Variant{}
                });
                return;
            }

            packet::Variant variant{};
            if (!variant.deserialize(ext_data)) {
                spdlog::warn("Failed to deserialize variant");
                return;
            }

            packet_callback_(PacketParser{
                ParseType::FromClient,
                from,
                to,
                game_update_packet,
                ext_data,
                variant
            });
        });
    }

    void free() override
    {
        delete this;
    }

    MessageCallback& get_message_callback() override
    {
        return message_callback_;
    }

    PacketCallback& get_packet_callback() override
    {
        return packet_callback_;
    }
};
}
