#pragma once
#include <enet/enet.h>
#include <span>
#include <cstddef>
#include <vector>

namespace network {
class ENetWrapper {
public:
    virtual ~ENetWrapper();

    void process();

    [[nodiscard]] bool is_valid() const { return host_ != nullptr; }

protected:
    explicit ENetWrapper(ENetHost* host);

    ENetWrapper(const ENetWrapper&) = delete;
    ENetWrapper& operator=(const ENetWrapper&) = delete;

    virtual void on_connect(ENetPeer* peer) = 0;
    virtual void on_receive(ENetPeer* peer, std::span<const std::byte> data) = 0;
    virtual void on_disconnect(ENetPeer* peer) = 0;

    ENetHost* host_;
};
}
