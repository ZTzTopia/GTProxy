#pragma once
#include <span>
#include <vector>

#include "../packet/packet_helper.hpp"

namespace network {
class IConnection {
public:
    virtual ~IConnection() = default;

    [[nodiscard]] virtual bool write(std::span<const std::byte> data, int channel = 0) const = 0;
    [[nodiscard]] virtual bool write(const std::vector<std::byte>& data, int channel = 0) const = 0;

    template <class Packet>
    bool write(Packet& packet) {
        return packet::PacketHelper::write(packet, *this);
    }

    [[nodiscard]] virtual bool is_connected() const = 0;
};
}
