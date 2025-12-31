#pragma once
#include <enet/enet.h>
#include <span>
#include <string>
#include <cstdint>

#include "enet_wrapper.hpp"
#include "connection.hpp"
#include "../core/config.hpp"
#include "../event/event.hpp"
#include "../packet/packet_decoder.hpp"
#include "../packet/packet_registry.hpp"

namespace network {
class Client final : public ENetWrapper, public IConnection {
public:
    Client(core::Config& config, event::Dispatcher& dispatcher);

    bool connect(const std::string& host, std::uint16_t port);

    [[nodiscard]] bool write(std::span<const std::byte> data, int channel = 0) const override;
    [[nodiscard]] bool write(const std::vector<std::byte>& data, int channel = 0) const override;

    void disconnect() const;
    void disconnect_now();

    [[nodiscard]] bool is_connected() const override;

    void flush() const;

protected:
    void on_connect(ENetPeer* peer) override;
    void on_receive(ENetPeer* peer, std::span<const std::byte> data) override;
    void on_disconnect(ENetPeer* peer) override;

private:
    static ENetHost* create_host();

private:
    core::Config& config_;

    event::Dispatcher& dispatcher_;
    ENetPeer* peer_;

    packet::PacketDecoder decoder_;
};
}
