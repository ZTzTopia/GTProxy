#include "server.hpp"

#include <spdlog/spdlog.h>
#include <stdexcept>

#include "../utils/network.hpp"

namespace network {
Server::Server(core::Config& config, event::Dispatcher& dispatcher)
    : ENetWrapper{ create_host(config.get_server_config().port) }
    , config_{ config }
    , dispatcher_{ dispatcher }
    , peer_{ nullptr }
{
    if (!host_) {
        throw std::runtime_error{"Failed to create proxy server host"};
    }

    spdlog::info(
        "Proxy server listening on port {} (max {} peers)",
        host_->address.port,
        host_->peerCount
    );
}

ENetHost* Server::create_host(std::uint16_t port)
{
    ENetAddress address{};
    address.host = ENET_HOST_ANY;
    address.port = port;

    ENetHost* host = enet_host_create(&address, 1, 2, 0, 0);
    if (!host) {
        return nullptr;
    }

    if (enet_host_compress_with_range_coder(host) != 0) {
        enet_host_destroy(host);
        return nullptr;
    }

    host->checksum = enet_crc32;
    host->usingNewPacketForServer = 1;

    return host;
}

void Server::on_connect(ENetPeer* peer)
{
    spdlog::info(
        "Client {}:{} connected to proxy server",
        network::format_ip_address(peer->address.host),
        peer->address.port
    );

    enet_peer_timeout(peer, 0, ENET_PEER_TIMEOUT_MAXIMUM / 2, 0);
    peer_ = peer;

    const event::ConnectionEvent evt{ event::Type::ClientConnect };
    dispatcher_.dispatch(evt);
}

void Server::on_receive(ENetPeer* peer, std::span<const std::byte> data)
{
    if (peer != peer_) {
        return;
    }

    if (data.size() < 4 || data.size() > 16384) {
        spdlog::warn("Received malformed packet from client (size {})", data.size());
        disconnect();
        return;
    }

    spdlog::info(
        "Received {} bytes from Growtopia client",
        data.size()
    );

    const auto packet{ decoder_.try_decode(data) };
    if (!packet) {
        const event::RawPacketEvent evt{ event::Type::ServerBoundPacket, data };
        dispatcher_.dispatch(evt);
        return;
    }

    const event::PacketEvent evt{ event::Type::ServerBoundPacket, packet };
    dispatcher_.dispatch(evt);
}

void Server::on_disconnect(ENetPeer* peer)
{
    if (peer != peer_) {
        return;
    }

    spdlog::info(
        "Client {}:{} disconnected from proxy server",
        network::format_ip_address(peer->address.host),
        peer->address.port
    );

    peer_ = nullptr;

    const event::ConnectionEvent evt{ event::Type::ClientDisconnect };
    dispatcher_.dispatch(evt);
}

bool Server::write(std::span<const std::byte> data, const int channel) const
{
    if (!peer_ || peer_->state != ENET_PEER_STATE_CONNECTED) {
        return false;
    }

    ENetPacket* packet = enet_packet_create(
        data.data(),
        data.size(),
        ENET_PACKET_FLAG_RELIABLE
    );

    if (enet_peer_send(peer_, channel, packet) != 0) {
        enet_packet_destroy(packet);
        return false;
    }

    return true;
}

bool Server::write(const std::vector<std::byte>& data, const int channel) const
{
    return write(std::span{ data.data(), data.size() }, channel);
}

void Server::disconnect() const
{
    if (peer_) {
        enet_peer_disconnect(peer_, 0);
    }
}

void Server::disconnect_now()
{
    if (!peer_) {
        return;
    }

    enet_peer_disconnect_now(peer_, 0);
    peer_ = nullptr;
}

bool Server::is_connected() const
{
    return peer_ && peer_->state == ENET_PEER_STATE_CONNECTED;
}

void Server::flush() const
{
    if (host_) {
        enet_host_flush(host_);
    }
}
}
