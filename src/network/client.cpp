#include "client.hpp"

#include <stdexcept>
#include <spdlog/spdlog.h>

#include "../packet/packet_event_registry.hpp"
#include "../utils/network.hpp"

namespace network {
Client::Client(core::Config& config, event::Dispatcher& dispatcher)
    : ENetWrapper{ create_host() }
    , config_{ config }
    , dispatcher_{ dispatcher }
    , peer_{ nullptr }
{
    if (!host_) {
        throw std::runtime_error{ "Failed to create proxy client host" };
    }

    spdlog::info("Proxy client ready to connect");
}

ENetHost* Client::create_host()
{
    ENetHost* host{ enet_host_create(nullptr, 1, 2, 0, 0) };
    if (!host) {
        return nullptr;
    }

    if (enet_host_compress_with_range_coder(host) != 0) {
        enet_host_destroy(host);
        return nullptr;
    }

    host->checksum = enet_crc32;
    host->usingNewPacket = 1;

    return host;
}

bool Client::connect(const std::string& host, std::uint16_t port)
{
    if (!host_) {
        return false;
    }

    ENetAddress address{};
    enet_address_set_host(&address, host.c_str());
    address.port = port;

    peer_ = enet_host_connect(host_, &address, 2, 0);
    return peer_ != nullptr;
}

void Client::on_connect(ENetPeer* peer)
{
    spdlog::info(
        "Proxy client connected to Growtopia server at {}:{}",
        network::format_ip_address(peer->address.host),
        peer->address.port
    );

    peer_ = peer;

    const event::ConnectionEvent evt{ event::Type::ServerConnect };
    dispatcher_.dispatch(evt);
}

void Client::on_receive(ENetPeer* peer, std::span<const std::byte> data)
{
    if (peer != peer_) {
        return;
    }

    if (data.size() < 4) {
        spdlog::warn("Received malformed packet from Growtopia server (size {})", data.size());
        disconnect();
        return;
    }

    auto pkt_log = spdlog::get("packet");
    pkt_log->info(
        "Received {} bytes from Growtopia server",
        data.size()
    );

    const auto decoded{ decoder_.decode(data) };
    if (!decoded.has_value()) {
        const event::RawPacketEvent evt{ event::Type::ClientBoundPacket, data };
        dispatcher_.dispatch(evt);
        return;
    }

    const auto& packet{ decoded.value() };
    if (
        const auto& registry{ packet::event_registry::PacketEventRegistry::instance() };
        registry.has_event(packet->id())
    ) {
        auto evt = registry.emit(
            dispatcher_,
            event::Direction::ClientBound,
            packet
        );

        if (evt && evt->canceled) {
            return;
        }
    }

    const event::PacketEvent evt{ event::Type::ClientBoundPacket, packet };
    dispatcher_.dispatch(evt);
}

void Client::on_disconnect(ENetPeer* peer)
{
    if (peer != peer_) {
        return;
    }

    spdlog::info(
        "Proxy client disconnected from Growtopia server at {}:{}",
        network::format_ip_address(peer->address.host),
        peer->address.port
    );

    peer_ = nullptr;

    const event::ConnectionEvent evt{ event::Type::ServerDisconnect };
    dispatcher_.dispatch(evt);
}

bool Client::write(std::span<const std::byte> data, const int channel) const
{
    if (!is_connected()) {
        return false;
    }

    /*auto pkt_log = spdlog::get("packet");
    pkt_log->debug(
        "Sending {} bytes to Growtopia server:{}",
        data.size(),
        spdlog::to_hex(data.begin(), data.end())
    );*/

    ENetPacket* packet{ enet_packet_create(
        data.data(),
        data.size(),
        ENET_PACKET_FLAG_RELIABLE
    ) };

    if (enet_peer_send(peer_, channel, packet) != 0) {
        enet_packet_destroy(packet);
        return false;
    }

    return true;
}

bool Client::write(const std::vector<std::byte>& data, const int channel) const
{
    return write(std::span{ data.data(), data.size() }, channel);
}

void Client::disconnect() const
{
    if (peer_) {
        enet_peer_disconnect(peer_, 0);
    }
}

void Client::disconnect_now()
{
    if (!peer_) {
        return;
    }

    enet_peer_disconnect_now(peer_, 0);
    peer_ = nullptr;
}

bool Client::is_connected() const
{
    return peer_ && peer_->state == ENET_PEER_STATE_CONNECTED;
}

void Client::flush() const
{
    if (host_) {
        enet_host_flush(host_);
    }
}
}
