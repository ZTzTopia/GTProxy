#include <eventpp/utilities/argumentadapter.h>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

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

    core_->get_event_dispatcher().appendListener(
        core::EventType::Connection,
        eventpp::argumentAdapter<void(const core::EventConnection&)>([&](const core::EventConnection& evt)
        {
            if (evt.from != core::EventFrom::FromClient) {
                return;
            }

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
        })
    );

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

    const core::EventConnection event_connection{ *player_ };
    event_connection.from = core::EventFrom::FromServer;
    core_->get_event_dispatcher().dispatch(event_connection);
}

void Client::on_receive(ENetPeer* peer, ENetPacket* packet)
{
    if (!player_) {
        enet_peer_disconnect(peer, 0);
        return;
    }

    player::Player* to_player{ core_->get_server()->get_player() };
    if (!to_player) {
        player_->disconnect();
        return;
    }

    ByteStream byte_stream{ reinterpret_cast<std::byte*>(packet->data), packet->dataLength };
    if (byte_stream.get_size() < 4 || byte_stream.get_size() > 786432 /* 768kb */) {
        player_->disconnect();
        return;
    }

    enet_packet_destroy(packet);

    packet::NetMessageType type{};
    if (!byte_stream.read(type)) {
        player_->disconnect();
        return;
    }

    if (type == packet::NET_MESSAGE_SERVER_HELLO) {
        packet::core::ServerHello server_hello{};
        packet::PacketHelper::send(server_hello, *to_player);
    }
    else if (type == packet::NET_MESSAGE_GENERIC_TEXT || type == packet::NET_MESSAGE_GAME_MESSAGE) {
        std::string message{};
        byte_stream.read(message, byte_stream.get_size() - sizeof(packet::NetMessageType) - 1);

        const core::EventMessage event_message{ *player_, *to_player, TextParse{ message } };
        event_message.from = core::EventFrom::FromServer;
        core_->get_event_dispatcher().dispatch(event_message);

        if (!event_message.canceled) {
            bool _ = to_player->send_packet(byte_stream.get_data(), 0);
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
        event_packet.from = core::EventFrom::FromServer;
        core_->get_event_dispatcher().dispatch(event_packet);

        if (!event_packet.canceled) {
            bool _ = to_player->send_packet(byte_stream.get_data(), 0);
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

    const core::EventDisconnection event_disconnection{ *player_ };
    event_disconnection.from = core::EventFrom::FromServer;
    core_->get_event_dispatcher().dispatch(event_disconnection);

    delete player_;
    player_ = nullptr;

    const player::Player* to_player{ core_->get_server()->get_player() };
    if (!to_player) {
        return;
    }

    enet_host_flush(host_); // Flush all outgoing packets before disconnecting
    to_player->disconnect();
}
}
