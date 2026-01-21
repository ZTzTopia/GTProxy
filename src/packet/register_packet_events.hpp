#pragma once
#include "packet_event_registry.hpp"
#include "game/server.hpp"
#include "game/world.hpp"
#include "message/chat.hpp"
#include "message/exit.hpp"
#include "message/input.hpp"
#include "message/server_hello.hpp"

namespace packet::event_registry {
inline void register_packet_events() {
    auto& registry{ PacketEventRegistry::instance() };

    registry.register_event(
        PacketId::ServerHello,
        make_event_builder<message::ServerHello, PacketId::ServerHello>()
    );

    registry.register_event(
        PacketId::Quit,
        make_event_builder<message::Quit, PacketId::Quit>()
    );
    registry.register_event(
        PacketId::QuitToExit,
        make_event_builder<message::QuitToExit, PacketId::QuitToExit>()
    );
    registry.register_event(
        PacketId::JoinRequest,
        make_event_builder<message::JoinRequest, PacketId::JoinRequest>()
    );
    registry.register_event(
        PacketId::ValidateWorld,
        make_event_builder<message::ValidateWorld, PacketId::ValidateWorld>()
    );
    registry.register_event(
        PacketId::Input,
        make_event_builder<message::Input, PacketId::Input>()
    );
    registry.register_event(
        PacketId::Log,
        make_event_builder<message::Log, PacketId::Log>()
    );

    registry.register_event(
        PacketId::Disconnect,
        make_event_builder<game::Disconnect, PacketId::Disconnect>()
    );

    registry.register_event(
        PacketId::OnSendToServer,
        make_event_builder<game::OnSendToServer, PacketId::OnSendToServer>()
    );

    registry.register_event(
        PacketId::OnSpawn,
        make_event_builder<game::OnSpawn, PacketId::OnSpawn>()
    );
    registry.register_event(
        PacketId::OnRemove,
        make_event_builder<game::OnRemove, PacketId::OnRemove>()
    );
}
}
