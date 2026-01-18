#pragma once
#include "packet_registry.hpp"
#include "register_packet_events.hpp"
#include "game/on_send_to_server.hpp"
#include "message/chat.hpp"
#include "message/exit.hpp"
#include "message/input.hpp"
#include "message/server_hello.hpp"

namespace packet {
inline bool register_all_packets()
{
    static bool registered{ false };
    if (registered) {
        return false;
    }

    registered = true;

    auto& registry{ PacketRegistry::instance() };

    registry.register_packet<message::ServerHello>();

    registry.register_packet<message::Quit>();
    registry.register_packet<message::QuitToExit>();
    registry.register_packet<message::JoinRequest>();
    registry.register_packet<message::ValidateWorld>();
    registry.register_packet<message::Input>();
    registry.register_packet<message::Log>();

    registry.register_packet<game::Disconnect>();

    registry.register_packet<game::OnSendToServer>();

    event_registry::register_packet_events();
    return true;
}
}
