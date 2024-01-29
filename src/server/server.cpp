#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>

#include "server.hpp"
#include "../client/client.hpp"
#include "../packet/packet_types.hpp"
#include "../utils/network.hpp"

namespace server {
Server::Server(core::Core* core)
    : ENetWrapper{ static_cast<enet_uint16>(core->get_config().get<unsigned int>("server.port")), 1 }
    , core_{ core }
    , player_{ nullptr }
{
    if (!host_) {
        throw std::runtime_error{ "Failed to create an ENet server host!" };
    }

    spdlog::info(
        "The server is up and running with port {} and {} peers can join!",
        host_->address.port,
        host_->peerCount
    );
}

Server::~Server()
{
    delete player_;
}

void Server::process()
{
    // Perform server processing here
    ENetWrapper::process();
}

void Server::on_connect(ENetPeer* peer)
{
    spdlog::info(
        "The server just got a new connection from the address {}:{}!",
        network::format_ip_address(peer->address.host),
        peer->address.port
    );

    // GOOD JOB GROWTOPIA TEAM! PLEASE MAKE YOUR CLIENTS HANG LONGER!!!
    enet_peer_timeout(peer, 0, 12000, 0);

    player_ = new player::Player{ peer };
    connect_callback_(*player_);
}

void Server::on_receive(ENetPeer* peer, ENetPacket* packet)
{
    ByteStream byte_stream{ reinterpret_cast<std::byte*>(packet->data), packet->dataLength };
    if (byte_stream.get_size() < 4 || byte_stream.get_size() > 16384 /* 16kb */) {
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

    const player::Player* to_player{ core_->get_client()->get_player() };
    if (!to_player) {
        enet_peer_disconnect(peer, 0);
        return;
    }

    if (type == packet::NET_MESSAGE_GENERIC_TEXT || type == packet::NET_MESSAGE_GAME_MESSAGE) {
        std::string message{};
        byte_stream.read(message, byte_stream.get_size() - sizeof(packet::NetMessageType) - 1);
        message_callback_(*player_, *to_player, message);
    }
    else if (type == packet::NET_MESSAGE_GAME_PACKET) {
        packet_callback_(*player_, *to_player, byte_stream.get_data());
    }
    else {
        spdlog::warn(
            "Got an unknown packet type coming in from the address {}:{}:",
            network::format_ip_address(peer->address.host),
            peer->address.port
        );
        spdlog::warn("\t{} ({})", magic_enum::enum_name(type), magic_enum::enum_integer(type));
    }

    bool _ = to_player->send_packet(byte_stream.get_data(), 0);
}

void Server::on_disconnect(ENetPeer* peer)
{
    spdlog::info(
        "The server just lost a connection from the address {}:{}!",
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
