#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>

#include "client.hpp"
#include "../packet/packet_helper.hpp"
#include "../packet/message/core.hpp"
#include "../server/server.hpp"
#include "../utils/hash.hpp"
#include "../utils/network.hpp"

namespace client {
Client::Client(core::Core* core)
    : ENetWrapper{ 1 }
    , core_{ core }
    , player_{ nullptr }
{
    if (!host_) {
        throw std::runtime_error{ "Failed to create an ENet client host!" };
    }

    core_->get_init_callback().append([&]()
    {
        core_->get_server()->get_connect_callback().append([&](const auto& player)
        {
            auto [address, port]{ core_->get_address() };
            player_ = new player::Player{ connect(address, port) };
            pre_connect_callback_(*player_);
        });

        core_->get_server()->get_disconnect_callback().append([&](const auto& player)
        {

        });

        core_->get_server()->get_receive_message_callback().append([&](const auto& player, const auto& text_parse)
        {
            return true;
        });
    });

    spdlog::info("The client is ready to connect to the server!");
}

Client::~Client()
{
    delete player_;
}

void Client::process()
{
    // Perform client processing here

    ENetWrapper::process();
}

void Client::on_connect(ENetPeer* peer)
{
    spdlog::info(
        "The client just connected to the server at {}:{}!",
        network::format_ip_address(peer->address.host),
        peer->address.port
    );

    connect_callback_(*player_);
}

void Client::on_receive(ENetPeer* peer, ENetPacket* packet)
{
    ByteStream byte_stream{ reinterpret_cast<std::byte*>(packet->data), packet->dataLength };
    if (byte_stream.get_size() < 4 || byte_stream.get_size() > 786432 /* 768kb */) {
        enet_peer_disconnect(peer, 0);
        return;
    }

    enet_packet_destroy(packet);

    if (!player_) {
        enet_peer_disconnect(peer, 0);
        return;
    }

    packet::NetMessageType type{};
    if (!byte_stream.read(type)) {
        enet_peer_disconnect(peer, 0);
        return;
    }

    if (type == packet::NET_MESSAGE_SERVER_HELLO) {
        player::Player* player{ core_->get_server()->get_player() };
        if (!player) {
            enet_peer_disconnect(peer, 0);
            return;
        }

        packet::core::ServerHello server_hello{};
        packet::PacketHelper::send(server_hello, *player);
    }
    else if (type == packet::NET_MESSAGE_GENERIC_TEXT || type == packet::NET_MESSAGE_GAME_MESSAGE) {
        std::string message{};
        byte_stream.read(message, byte_stream.get_size() - sizeof(packet::NetMessageType) - 1);

        spdlog::debug(
            "Got a message coming in from the address {}:{}!",
            network::format_ip_address(peer->address.host),
            peer->address.port
        );
        spdlog::debug("\tMessage: {}", message);

        const TextParse text_parse{ message };
        receive_message_callback_.forEachIf([&](const auto& callback)
        {
            return !callback(*player_, text_parse);
        });
    }
    else {
        spdlog::warn(
            "Got an unknown packet type coming in from the address {}:{}!",
            network::format_ip_address(peer->address.host),
            peer->address.port
        );
        spdlog::warn("\tPacket Type: {}", magic_enum::enum_name(type));
    }
}

void Client::on_disconnect(ENetPeer* peer)
{
    spdlog::info(
        "The client just disconnected from the server at {}:{}!",
        network::format_ip_address(peer->address.host),
        peer->address.port
    );

    if (!player_) {
        return;
    }

    disconnect_callback_(*player_);

    delete player_;
    player_ = nullptr;
}
}
