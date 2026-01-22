#pragma once
#include <type_traits>
#include <concepts>
#include <span>

#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>

#include "packet_types.hpp"
#include "packet_id.hpp"
#include "payload.hpp"
#include "../utils/byte_stream.hpp"

namespace packet {
template<typename T>
concept NetworkSender = requires(T& sender, const std::span<const std::byte>& data, int channel) {
    { sender.write(data, channel) } -> std::convertible_to<bool>;
};

class IPacket : public std::enable_shared_from_this<IPacket> {
public:
    virtual ~IPacket() = default;

    [[nodiscard]] virtual PacketId id() const = 0;
    [[nodiscard]] virtual int channel() const { return 0; }
    [[nodiscard]] virtual bool read(const Payload& payload) = 0;
    [[nodiscard]] virtual Payload write() = 0;
};

template <PacketId Id, NetMessageType MsgType = NET_MESSAGE_GAME_MESSAGE, int Channel = 0>
struct TextPacket : IPacket {
    static constexpr PacketId ID = Id;
    static constexpr NetMessageType MESSAGE_TYPE = MsgType;
    static constexpr int CHANNEL = Channel;
    using IsTextPacket = std::true_type;

    [[nodiscard]] PacketId id() const override { return ID; }
    [[nodiscard]] int channel() const override { return CHANNEL; }

    [[nodiscard]] std::shared_ptr<IPacket> shared_from_this() {
        return std::shared_ptr<IPacket>(this, [](IPacket*) {});
    }
};

template <PacketId Id, PacketType PktType, int Channel = 0>
struct GamePacket : IPacket {
    static constexpr PacketId ID = Id;
    static constexpr NetMessageType MESSAGE_TYPE = NET_MESSAGE_GAME_PACKET;
    static constexpr PacketType PACKET_TYPE = PktType;
    static constexpr int CHANNEL = Channel;
    using IsGamePacket = std::true_type;

    [[nodiscard]] PacketId id() const override { return ID; }
    [[nodiscard]] int channel() const override { return CHANNEL; }

    [[nodiscard]] std::shared_ptr<IPacket> shared_from_this() {
        return std::shared_ptr<IPacket>(this, [](IPacket*) {});
    }
};

template <PacketId Id, int Channel = 0>
struct VariantPacket : IPacket {
    static constexpr PacketId ID = Id;
    static constexpr NetMessageType MESSAGE_TYPE = NET_MESSAGE_GAME_PACKET;
    static constexpr PacketType PACKET_TYPE = PACKET_CALL_FUNCTION;
    static constexpr int CHANNEL = Channel;
    using IsVariantPacket = std::true_type;

    [[nodiscard]] PacketId id() const override { return ID; }
    [[nodiscard]] int channel() const override { return CHANNEL; }

    [[nodiscard]] std::shared_ptr<IPacket> shared_from_this() {
        return std::shared_ptr<IPacket>(this, [](IPacket*) {});
    }
};

struct PacketHelper {
    static std::vector<std::byte> serialize(const Payload& payload)
    {
        ByteStream byte_stream{};

        if (const auto* text = get_payload_if<TextPayload>(payload)) {
            byte_stream.write(magic_enum::enum_underlying(text->message_type));
            byte_stream.write(text->data.get_raw(), false);
        }
        else if (const auto* game = get_payload_if<GamePayload>(payload)) {
            byte_stream.write(magic_enum::enum_underlying(NET_MESSAGE_GAME_PACKET));

            GameUpdatePacket header = game->packet;
            if (!game->extra.empty()) {
                header.flags.extended = 1;
                header.data_size = static_cast<uint32_t>(game->extra.size());
            }

            byte_stream.write(header);
            byte_stream.write_data(game->extra.data(), game->extra.size());
        }
        else if (const auto* var = get_payload_if<VariantPayload>(payload)) {
            byte_stream.write(magic_enum::enum_underlying(NET_MESSAGE_GAME_PACKET));

            GameUpdatePacket game_packet{ var->game_packet };
            game_packet.type = PACKET_CALL_FUNCTION;

            const auto ext_data = var->variant.serialize();
            game_packet.flags.extended = 1;
            game_packet.data_size = static_cast<uint32_t>(ext_data.size());

            byte_stream.write(game_packet);
            byte_stream.write_data(ext_data.data(), ext_data.size());
        }
        
        return byte_stream.get_data();
    }

    static std::vector<std::byte> serialize(IPacket& packet)
    {
        return serialize(packet.write());
    }

    static bool write(IPacket& packet, NetworkSender auto& sender)
    {
        auto data{ serialize(packet) };
        if (data.empty()) {
            return false;
        }

        data.push_back(static_cast<std::byte>(0x00));
        return sender.write(data, packet.channel());
    }

    template <class Packet, NetworkSender Sender>
    requires (std::derived_from<Packet, IPacket>)
    static bool write(Packet& packet, Sender& sender)
    {
        auto data{ serialize(packet) };
        if (data.empty()) {
            return false;
        }

        data.push_back(static_cast<std::byte>(0x00));
        return sender.write(data, Packet::CHANNEL);
    }
};
}
