#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

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

    const core::EventConnection event_connection{ *player_ };
    event_connection.from = core::EventFrom::FromClient;
    core_->get_event_dispatcher().dispatch(event_connection);
}

void Server::on_receive(ENetPeer* peer, ENetPacket* packet)
{
    if (!player_) {
        enet_peer_disconnect(peer, 0);
        return;
    }

    const player::Player* to_player{ core_->get_client()->get_player() };
    if (!to_player) {
        player_->disconnect();
        return;
    }

    ByteStream byte_stream{ reinterpret_cast<std::byte*>(packet->data), packet->dataLength };
    if (byte_stream.get_size() < 4 || byte_stream.get_size() > 16384 /* 16kb */) {
        player_->disconnect();
        return;
    }

    enet_packet_destroy(packet);

    packet::NetMessageType type{};
    if (!byte_stream.read(type)) {
        player_->disconnect();
        return;
    }

    if (type == packet::NET_MESSAGE_GENERIC_TEXT || type == packet::NET_MESSAGE_GAME_MESSAGE) {
        std::string message{};
        byte_stream.read(message, byte_stream.get_size() - sizeof(packet::NetMessageType) - 1);

        const core::EventMessage event_message{ *player_, *to_player, TextParse{ message } };
        event_message.from = core::EventFrom::FromClient;
        core_->get_event_dispatcher().dispatch(event_message);

        if (!event_message.canceled) {
            bool _ = to_player->send_packet(byte_stream.get_data(), 0);
        }

        if (message.find("action|quit") != std::string::npos) {
            player_->disconnect();
        }
    }
    else if (type == packet::NET_MESSAGE_GAME_PACKET) {
        packet::GameUpdatePacket game_update_packet{};
        byte_stream.read(game_update_packet);

        std::vector<std::byte> ext_data{};
        if (game_update_packet.data_size > 0) {
            byte_stream.read_vector(ext_data, game_update_packet.data_size);
        }

        const core::EventPacket event_packet{ *player_, *to_player, game_update_packet, ext_data };
        event_packet.from = core::EventFrom::FromClient;
        core_->get_event_dispatcher().dispatch(event_packet);

        if (!event_packet.canceled) {
            bool _ = to_player->send_packet(byte_stream.get_data(), 0);
        }

        if (game_update_packet.type == packet::PACKET_DISCONNECT) {
            // Because Growtopia's client is force recreate the ENetHost when the client is
            // disconnected, we need to disconnect the client immediately.
            player_->disconnect_now();
            on_disconnect(peer);
        }
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

    const core::EventDisconnection event_disconnection{ *player_ };
    event_disconnection.from = core::EventFrom::FromClient;
    core_->get_event_dispatcher().dispatch(event_disconnection);

    delete player_;
    player_ = nullptr;

    const player::Player* to_player{ core_->get_client()->get_player() };
    if (!to_player) {
        return;
    }

    enet_host_flush(host_); // Flush all outgoing packets before disconnecting
    to_player->disconnect();
}
}
