#pragma once
#include <cstddef>
#include <span>
#include <enet/enet.h>

#include "../utils/types.hpp"

namespace network {
class ENetWrapper : utils::types::Immobile {
public:
    virtual ~ENetWrapper();

    void process();

    [[nodiscard]] bool is_valid() const { return host_ != nullptr; }

protected:
    explicit ENetWrapper(ENetHost* host);

    virtual void on_connect(ENetPeer* peer) = 0;
    virtual void on_receive(ENetPeer* peer, std::span<const std::byte> data) = 0;
    virtual void on_disconnect(ENetPeer* peer) = 0;

protected:
    ENetHost* host_;
};
}
