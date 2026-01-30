#pragma once
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <fmt/format.h>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "packet_helper.hpp"
#include "packet_id.hpp"
#include "generic_packets.hpp"
#include "../utils/singleton.hpp"

namespace packet {
using PacketFactory = std::function<std::shared_ptr<IPacket>()>;

inline std::string packet_name(const PacketId id) {
    if (const auto name = magic_enum::enum_name(id); !name.empty()) {
        return std::string{name};
    }

    return fmt::format("PacketId({:#08x})", static_cast<uint32_t>(id));
}

class PacketRegistry : public utils::Singleton<PacketRegistry> {
public:
    template<typename T>
    void register_packet()
    {
        static_assert(
            std::is_base_of_v<IPacket, T>,
            "Registered type must derive from IPacket"
        );

        spdlog::debug("Registering packet: {}", packet_name(T::ID));
        registry_[T::ID] = [] { return std::make_shared<T>(); };
    }

    [[nodiscard]] std::shared_ptr<IPacket> create(const PacketId id) const
    {
        if (const auto it{ registry_.find(id) }; it != registry_.end()) {
            return it->second();
        }

        return nullptr;
    }

    [[nodiscard]] std::shared_ptr<IPacket> create(const Payload& payload) const
    {
        const PacketId id{ derive_packet_id(payload) };

        if (id != PacketId::Unknown) {
            if (auto packet{ create(id) }) {
                if (!packet->read(payload)) {
                    spdlog::warn("Failed to read packet {}", packet_name(id));
                    return nullptr;
                }
                return packet;
            }
        }

        if (const auto* text = get_payload_if<TextPayload>(payload)) {
            auto generic = std::make_shared<GenericTextPacket>();
            if (generic->read(payload)) {
                return generic;
            }
        }
        else if (const auto* var = get_payload_if<VariantPayload>(payload)) {
            if (auto generic = std::make_shared<GenericVariantPacket>(); generic->read(payload)) {
                return generic;
            }
        }
        else if (const auto* game = get_payload_if<GamePayload>(payload)) {
            if (auto generic = std::make_shared<GenericGamePacket>(); generic->read(payload)) {
                return generic;
            }
        }

        return nullptr;
    }

    [[nodiscard]] bool is_registered(const PacketId id) const
    {
        return registry_.contains(id);
    }

private:
    std::unordered_map<PacketId, PacketFactory> registry_;
};
}

