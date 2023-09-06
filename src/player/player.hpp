#pragma once
#include <vector>
#include <enet/enet.h>

#include "../packet/message/core.hpp"

namespace player {
class Player  {
public:
    explicit Player(ENetPeer* peer);
    ~Player() = default;

    bool send_packet(const std::vector<std::byte>& data, int channel);

private:
    ENetPeer* peer_;
};
}
