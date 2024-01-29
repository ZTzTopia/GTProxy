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

    core_->get_init_callback().append([&]
    {
        core_->get_server()->get_connect_callback().append([&](const  player::Player&)
        {
            const auto ext{ core_->get_extension(0x153bd697) };
            if (!ext) {
                spdlog::warn("The web server extension is not loaded!");
                spdlog::warn("Trying to using config address and port instead...");

                const core::Config config{ core_->get_config() };
                player_ = new player::Player{
                    connect(
                        config.get<std::string>("server.address"),
                        config.get<unsigned int>("server.port")
                    )
                };
                return;
            }

            player_ = new player::Player{
                connect(
                    ext->call_method<std::string>("get_server_address"),
                    ext->call_method<uint16_t>("get_server_port")
                )
            };
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

    player::Player* to_player{ core_->get_server()->get_player() };
    if (!to_player) {
        enet_peer_disconnect(peer, 0);
        return;
    }

    if (type == packet::NET_MESSAGE_SERVER_HELLO) {
        packet::core::ServerHello server_hello{};
        packet::PacketHelper::send(server_hello, *to_player);
    }
    else if (type == packet::NET_MESSAGE_GENERIC_TEXT || type == packet::NET_MESSAGE_GAME_MESSAGE) {
        std::string message{};
        byte_stream.read(message, byte_stream.get_size() - sizeof(packet::NetMessageType) - 1);
        message_callback_(*player_, *to_player, message);
    }
    else {
        spdlog::warn(
            "Got an unknown packet type coming in from the address {}:{}:",
            network::format_ip_address(peer->address.host),
            peer->address.port
        );
        spdlog::warn("\t{} ({})", magic_enum::enum_name(type), magic_enum::enum_integer(type));
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
