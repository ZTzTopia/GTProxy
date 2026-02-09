#pragma once

#include <optional>
#include <span>
#include <string_view>

#include "packet_registry.hpp"
#include "packet_types.hpp"
#include "payload.hpp"
#include "../core/config.hpp"
#include "../utils/byte_stream.hpp"

namespace packet {
class PacketDecoder {
public:
    [[nodiscard]] static std::optional<std::shared_ptr<IPacket>> decode(
        std::span<const std::byte> data,
        const core::Config::LogConfig& log_config,
        std::string_view direction
    );
};
}
