#include <magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>

#include "server.hpp"
#include "../packet/packet_types.hpp"

namespace server {
Server::Server(core::Core* core)
    : ENetWrapper{ static_cast<enet_uint16>(core->get_config().get<unsigned int>("server.port")), 1 }
    , core_{ core }
    , http_{ core }
    , player_{ nullptr }
{
    if (!host_) {
        throw std::runtime_error{ "Failed to create an ENet server host!" };
    }

    spdlog::info(
        "The server is up and running with port {} and {} peers can join!",
        core_->get_config().get<unsigned int>("server.port"),
        host_->peerCount
    );

    http_.listen("0.0.0.0", 443);
}

Server::~Server()
{
    delete player_;
    http_.stop();
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
        peer->address.host,
        peer->address.port
    );

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

    if (type == packet::NET_MESSAGE_GENERIC_TEXT || type == packet::NET_MESSAGE_GAME_MESSAGE) {
        std::string message{};
        byte_stream.read(message, byte_stream.get_size() - sizeof(packet::NetMessageType) - 1);

        spdlog::debug("Got a message coming in from the address {}:{}!", peer->address.host, peer->address.port);
        spdlog::debug("\tMessage: {}", message);

        const TextParse text_parse{ message };
        receive_message_callback_.forEachIf([&](const auto& callback)
        {
            return !callback(*player_, text_parse);
        });
    }
}

void Server::on_disconnect(ENetPeer* peer)
{
    spdlog::info(
        "The server just lost a connection from the address {}:{}!",
        peer->address.host,
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
