#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>

#include "client.hpp"
#include "../packet/packet_helper.hpp"
#include "../packet/message/core.hpp"
#include "../server/server.hpp"
#include "../utils/network.hpp"

namespace client {
Client::Client(core::Core* core)
    : core_{ core }
    , player_{ nullptr }
{
    host_ = enet_host_create(nullptr, 1, 2, 0, 0);
    if (!host_) {
        return;
    }

    if (enet_host_compress_with_range_coder(host_) != 0) {
        return;
    }

    host_->checksum = enet_crc32;
    host_->usingNewPacket = 1;

    core_->get_event_dispatcher().appendListener(
        core::EventType::Connection,
        [&](const core::EventConnection& evt)
        {
            if (evt.from != core::EventFrom::FromClient) {
                return;
            }

            if (const auto ext{ core_->get_extension(0x153bd697) }; ext) {
                return;
            }

            spdlog::warn("The web server extension is not loaded!");
            spdlog::warn("Trying to using config address and port instead...");

            const core::Config config{ core_->get_config() };
            std::ignore = connect(
                config.get<std::string>("server.address"),
                config.get<unsigned int>("server.port")
            );
        }
    );

    spdlog::info("The client is ready to connect to the server!");
}

Client::~Client()
{
    enet_host_destroy(host_);
    delete player_;
}

ENetPeer* Client::connect(const std::string& host, const enet_uint16 port) const
{
    if (!host_) {
        return nullptr;
    }

    ENetAddress address{};
    enet_address_set_host(&address, host.c_str());
    address.port = port;

    return enet_host_connect(host_, &address, 2, 0);
}

void Client::process()
{
    // Perform client processing here
    if (!host_) {
        return;
    }

    ENetEvent ev{};
    while (enet_host_service(host_, &ev, 16) > 0) {
        switch (ev.type) {
        case ENET_EVENT_TYPE_CONNECT:
            on_connect(ev.peer);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            on_disconnect(ev.peer);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            on_receive(ev.peer, ev.packet);
            break;
        default:
            break;
        }
    }
}

void Client::on_connect(ENetPeer* peer)
{
    spdlog::info(
        "The client just connected to the server at {}:{}!",
        network::format_ip_address(peer->address.host),
        peer->address.port
    );

    player_ = new player::Player{ peer };

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

        TextParse text_parse{ message };

        if (core_->get_config().get<bool>("log.printMessage")) {
            spdlog::info("Incoming message from server:");
            for (const auto& key_value : text_parse.get_key_values()) {
                spdlog::info("\t{}", key_value);
            }
        }

        const core::EventMessage event_message{ *player_, *to_player, text_parse };
        event_message.from = core::EventFrom::FromServer;
        core_->get_event_dispatcher().dispatch(event_message);

        if (!event_message.canceled) {
            std::ignore = to_player->send_packet(byte_stream.get_data(), 0);
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

        if (core_->get_config().get<bool>("log.printGameUpdatePacket")) {
            spdlog::info(
                "Incoming GameUpdatePacket {} ({}) from server: {:p}\n",
                magic_enum::enum_name(game_update_packet.type),
                magic_enum::enum_integer(game_update_packet.type),
                spdlog::to_hex(byte_stream.get_data())
            );
        }

        if (!event_packet.canceled) {
            std::ignore = to_player->send_packet(byte_stream.get_data(), 0);
        }
    }
    else {
        spdlog::warn(
            "Got an unknown packet type coming in from the address {}:{}:",
            network::format_ip_address(peer->address.host),
            peer->address.port
        );
        spdlog::warn("\t{} ({})", magic_enum::enum_name(type), magic_enum::enum_integer(type));
        std::ignore = to_player->send_packet(byte_stream.get_data(), 0);
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
    to_player->disconnect_now();
    core_->get_server()->on_disconnect(peer);
}
}
