#pragma once
#include <vector>

#include "../../event/event.hpp"
#include "../../packet/game/world.hpp"
#include "../../world/world.hpp"
#include "../../player/player.hpp"

namespace core {

class WorldHandler {
public:
    explicit WorldHandler(event::Dispatcher& dispatcher)
        : dispatcher_{ dispatcher }
    {
        setup_join_request_handler();
        setup_on_spawn_handler();
        setup_on_remove_handler();
    }

private:
    void setup_join_request_handler()
    {
        constexpr auto join_request_type{ event::packet_event_type(packet::PacketId::JoinRequest) };
        handles_.emplace_back(
            dispatcher_,
            join_request_type,
            dispatcher_.appendListener(join_request_type, [](const event::Event& event) {
                if (
                    const auto evt{ dynamic_cast<const event::TypedPacketEvent<packet::PacketId::JoinRequest>*>(&event) };
                    !evt || evt->direction != event::Direction::ServerBound
                ) {
                    return;
                }

                world::World::instance().clear();
            })
        );
    }

    void setup_on_spawn_handler()
    {
        constexpr auto on_spawn_type{ event::packet_event_type(packet::PacketId::OnSpawn) };
        handles_.emplace_back(
            dispatcher_,
            on_spawn_type,
            dispatcher_.appendListener(on_spawn_type, [](const event::Event& event) {
                const auto evt{ dynamic_cast<const event::TypedPacketEvent<packet::PacketId::OnSpawn>*>(&event) };
                if (!evt || evt->direction != event::Direction::ClientBound) {
                    return;
                }

                const auto pkt{ evt->get<packet::game::OnSpawn>() };
                if (!pkt) {
                    return;
                }

                const auto player{ player::Player::from_on_spawn(*pkt) };
                world::World::instance().add_player(player);
            })
        );
    }

    void setup_on_remove_handler()
    {
        constexpr auto on_remove_type{ event::packet_event_type(packet::PacketId::OnRemove) };
        handles_.emplace_back(
            dispatcher_,
            on_remove_type,
            dispatcher_.appendListener(on_remove_type, [](const event::Event& event) {
                const auto evt{ dynamic_cast<const event::TypedPacketEvent<packet::PacketId::OnRemove>*>(&event) };
                if (!evt || evt->direction != event::Direction::ClientBound) {
                    return;
                }

                const auto pkt{ evt->get<packet::game::OnRemove>() };
                if (!pkt) {
                    return;
                }

                world::World::instance().remove_player(pkt->net_id);
            })
        );
    }

private:
    event::Dispatcher& dispatcher_;
    std::vector<event::ScopedHandle> handles_;
};

}
