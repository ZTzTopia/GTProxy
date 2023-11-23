#include <magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>

#include "client.hpp"
#include "../server/server.hpp"
#include "../utils/hash.hpp"

namespace client {
Client::Client(core::Core* core)
    : ENetWrapper{ 1 }
    , core_{ core }
    , player_{ nullptr }
{
    if (!host_) {
        throw std::runtime_error{ "Failed to create an ENet client host!" };
    }
}

Client::~Client()
{
    delete player_;
}

void Client::process()
{
    // Perform client processing here

    ENetEvent ev{};
    enet_host_check_events(host_, &ev);

    if (ev.type == ENET_EVENT_TYPE_CONNECT && ev.peer->data == nullptr) {
        ev.peer->data = reinterpret_cast<void*>(0xdeadc0de);
        return;
    }

    ENetWrapper::process();
}

void Client::on_connect(ENetPeer* peer)
{
    spdlog::info(
        "The client just connected to the server at {}:{}!",
        peer->address.host,
        peer->address.port
    );

    player_ = new player::Player{ peer };
}

void Client::on_receive(ENetPeer* peer, ENetPacket* packet)
{
    ByteStream byte_stream{ reinterpret_cast<std::byte*>(packet->data), packet->dataLength };
    if (byte_stream.get_size() < 4 || byte_stream.get_size() > 786432 /* 768kb */) {
        enet_peer_disconnect(peer, 0);
        return;
    }

    if (!player_) {
        enet_peer_disconnect(peer, 0);
        return;
    }

    if (packet::NetMessageType type{}; !byte_stream.read(type)) {
        enet_peer_disconnect(peer, 0);
        return;
    }

    enet_packet_destroy(packet);
}

void Client::on_disconnect(ENetPeer* peer)
{
    spdlog::info(
        "The client just disconnected from the server at {}:{}!",
        peer->address.host,
        peer->address.port
    );

    if (!player_) {
        return;
    }

    delete player_;
    player_ = nullptr;
}
}
