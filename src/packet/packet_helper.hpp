#pragma once
#include <type_traits>
#include <concepts>
#include <span>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>

#include "packet_types.hpp"
#include "packet_variant.hpp"
#include "../utils/byte_stream.hpp"
#include "../utils/text_parse.hpp"

// What the heck im even doing here
#include "../event/event.hpp"

// What the heck im even doing here
namespace packet {
// WOW this is so cool
template<typename T>
concept NetworkSender = requires(T& sender, const std::span<const std::byte>& data, int channel) {
    { sender.write(data, channel) } -> std::convertible_to<bool>;
    { sender.is_connected() } -> std::convertible_to<bool>;
};

class IPacket : public std::enable_shared_from_this<IPacket> {
public:
    virtual ~IPacket() = default;

    [[nodiscard]] virtual NetMessageType message_type() const = 0;
    [[nodiscard]] virtual PacketType packet_type() const = 0;
    [[nodiscard]] virtual int channel() const = 0;

    virtual void dispatch(event::Dispatcher& dispatcher, event::Type type) = 0;

    // TODO: Return false on failure instead of throwing
    [[nodiscard]] virtual bool read(const TextParse&) { throw std::runtime_error("Read not implemented"); }
    [[nodiscard]] virtual bool read(const GameUpdatePacket&, const std::vector<std::byte>&) { throw std::runtime_error("Read not implemented"); }
    [[nodiscard]] virtual bool read(const PacketVariant&) { throw std::runtime_error("Read not implemented"); }

    // TODO: Return false on failure instead of throwing
    virtual void write() { throw std::runtime_error("Write not implemented"); }
    virtual void write(ByteStream<>&) { throw std::runtime_error("Write not implemented"); }
    virtual void write(GameUpdatePacket&, std::vector<std::byte>&) { throw std::runtime_error("Write not implemented"); }
};

template <typename T, NetMessageType MsgType, int Channel = 0>
struct NetMessage : IPacket {
    ~NetMessage() override = default;

    static constexpr NetMessageType MESSAGE_TYPE = MsgType;
    static constexpr int CHANNEL = Channel;
    using IsNetMessage = std::true_type;

    [[nodiscard]] NetMessageType message_type() const override { return MESSAGE_TYPE; }
    [[nodiscard]] PacketType packet_type() const override { return PACKET_MAX; }
    [[nodiscard]] int channel() const override { return CHANNEL; }

    void dispatch(event::Dispatcher& dispatcher, event::Type type) override
    {
        const event::PacketEvent<T> evt{ type, std::static_pointer_cast<T>(shared_from_this()) };
        dispatcher.dispatch(evt);
    }
};

template <typename T, PacketType PktType, int Channel = 0>
struct NetPacket : IPacket {
    ~NetPacket() override = default;

    static constexpr NetMessageType MESSAGE_TYPE = NET_MESSAGE_GAME_PACKET;
    static constexpr PacketType PACKET_TYPE = PktType;
    static constexpr int CHANNEL = Channel;
    using IsNetPacket = std::true_type;

    [[nodiscard]] NetMessageType message_type() const override { return MESSAGE_TYPE; }
    [[nodiscard]] PacketType packet_type() const override { return PACKET_TYPE; }
    [[nodiscard]] int channel() const override { return CHANNEL; }

    void dispatch(event::Dispatcher& dispatcher, event::Type type) override
    {
        const event::PacketEvent<T> evt{ type, std::static_pointer_cast<T>(shared_from_this()) };
        dispatcher.dispatch(evt);
    }
};

template <typename T>
auto test_is_net_packet(int) -> std::bool_constant<T::IsNetPacket::value>;
template <typename T>
std::false_type test_is_net_packet(...);

template <typename T>
using is_net_packet = decltype(test_is_net_packet<T>(0));

template <typename T>
auto test_is_net_message(int) -> std::bool_constant<T::IsNetMessage::value>;
template <typename T>
std::false_type test_is_net_message(...);

template <typename T>
using is_net_message = decltype(test_is_net_message<T>(0));

struct PacketHelper {
    static std::vector<std::byte> serialize(IPacket& packet)
    {
        ByteStream byte_stream{};
        byte_stream.write(magic_enum::enum_underlying(packet.message_type()));

        if (packet.message_type() != NET_MESSAGE_GAME_PACKET) {
            packet.write(byte_stream);
        }
        else {
            GameUpdatePacket game_packet{};
            std::vector<std::byte> ext_data{};

            packet.write(game_packet, ext_data);

            game_packet.type = packet.packet_type();
            if (!ext_data.empty()) {
                game_packet.flags.extended = 1;
                game_packet.data_size = static_cast<std::uint32_t>(ext_data.size());
            }

            byte_stream.write(game_packet);
            byte_stream.write_data(ext_data.data(), ext_data.size());
        }

        return byte_stream.get_data();
    }

    template <class Packet>
    static std::vector<std::byte> serialize(Packet& packet)
    {
        if constexpr (!is_net_message<Packet>::value && !is_net_packet<Packet>::value) {
            return {};
        }

        ByteStream byte_stream{};
        byte_stream.write(magic_enum::enum_underlying(Packet::MESSAGE_TYPE));

        if constexpr (is_net_message<Packet>::value) {
            spdlog::debug("Serializing net message of type {}", magic_enum::enum_name(Packet::MESSAGE_TYPE));
            packet.write(byte_stream);
        }
        else if constexpr (is_net_packet<Packet>::value) {
            spdlog::debug("Serializing net packet of type {}", magic_enum::enum_name(Packet::PACKET_TYPE));
            GameUpdatePacket game_packet{};
            std::vector<std::byte> ext_data{};

            packet.write(game_packet, ext_data);

            game_packet.type = Packet::PACKET_TYPE;
            if (!ext_data.empty()) {
                game_packet.flags.extended = 1;
                game_packet.data_size = static_cast<std::uint32_t>(ext_data.size());
            }

            byte_stream.write(game_packet);
            byte_stream.write_data(ext_data.data(), ext_data.size());
        }

        return byte_stream.get_data();
    }

    static bool write(IPacket& packet, NetworkSender auto& sender)
    {
        auto data{ serialize(packet) };
        if (data.empty()) {
            return false;
        }

        data.push_back(static_cast<std::byte>(0x00));

        spdlog::debug(
            "Sending {} on channel {}: {:p}",
            magic_enum::enum_name(packet.message_type()),
            packet.channel(),
            spdlog::to_hex(data)
        );
        return sender.write(data, packet.channel());
    }

    template <class Packet, NetworkSender Sender>
    requires (is_net_message<Packet>::value || is_net_packet<Packet>::value)
    static bool write(Packet& packet, Sender& sender)
    {
        auto data{ serialize(packet) };
        if (data.empty()) {
            return false;
        }

        data.push_back(static_cast<std::byte>(0x00));

        spdlog::debug(
            "Sending {} on channel {}: {:p}",
            magic_enum::enum_name(Packet::MESSAGE_TYPE),
            Packet::CHANNEL,
            spdlog::to_hex(data)
        );
        return sender.write(data, Packet::CHANNEL);
    }
};
}
