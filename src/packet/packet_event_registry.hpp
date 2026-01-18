#pragma once
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>

#include "packet_id.hpp"
#include "../event/event.hpp"

namespace packet::event_registry {
using PacketEventBuilder = std::function<std::shared_ptr<event::Event>(
    event::PriorityEventDispatcher&,
    event::Direction,
    std::shared_ptr<IPacket>
)>;

class PacketEventRegistry {
public:
    static PacketEventRegistry& instance()
    {
        static PacketEventRegistry registry;
        return registry;
    }

    void register_event(PacketId id, PacketEventBuilder builder)
    {
        builders_[id] = std::move(builder);
    }

    [[nodiscard]] bool has_event(PacketId id) const
    {
        return builders_.find(id) != builders_.end();
    }

    [[nodiscard]] std::shared_ptr<event::Event> emit(
        event::PriorityEventDispatcher& dispatcher,
        const event::Direction direction,
        const std::shared_ptr<IPacket>& packet
    ) const {
        const auto it{ builders_.find(packet->id()) };
        if (it == builders_.end()) {
            return nullptr;
        }

        return it->second(dispatcher, direction, packet);
    }

private:
    PacketEventRegistry() = default;

    std::unordered_map<PacketId, PacketEventBuilder> builders_;
};

template<typename PacketType, PacketId PacketTypeId>
PacketEventBuilder make_event_builder()
{
    return [](
        event::PriorityEventDispatcher& dispatcher,
        const event::Direction direction,
        const std::shared_ptr<IPacket>& packet
    ) -> std::shared_ptr<event::Event> {
        auto typed_packet{ std::static_pointer_cast<PacketType>(packet) };
        auto evt = std::make_shared<event::TypedPacketEvent<PacketTypeId>>(
            direction,
            std::move(typed_packet)
        );
        dispatcher.dispatch(*evt);
        return evt;
    };
}
}
