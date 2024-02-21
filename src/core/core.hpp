#pragma once
#include <eventpp/hetereventdispatcher.h>
#include <eventpp/utilities/eventmaker.h>

#include "config.hpp"
#include "../extension/extension.hpp"
#include "../packet/packet_types.hpp"
#include "../player/player.hpp"
#include "../utils/text_parse.hpp"

namespace server {
class Server;
}

namespace client {
class Client;
}

namespace core {
enum class EventType {
    Init,
    Tick,
    Connection,
    Disconnection,
    Message,
    Packet
};

enum class EventFrom {
    FromAny,
    FromClient,
    FromServer
};

class Event {
public:
    explicit Event(const EventType type)
        : type(type)
        , from(EventFrom::FromAny)
        , canceled(false)
    {

    }

    Event(const EventType type, const EventFrom from)
        : type(type)
        , from(from)
        , canceled(false)
    {

    }

    EventType type;
    mutable EventFrom from;
    mutable bool canceled;
};

#define G(type, name) (type, get_ ## name)
#define GS(type, name) (type, get_ ## name, set_ ## name)

EVENTPP_MAKE_EMPTY_EVENT(
    EventInit, Event, (EventType::Init, EventFrom::FromAny)
);

EVENTPP_MAKE_EMPTY_EVENT(
    EventTick, Event, (EventType::Tick, EventFrom::FromAny)
);

EVENTPP_MAKE_EVENT(
    EventConnection, Event, (EventType::Connection, EventFrom::FromAny),
    G(player::Player, player)
);

EVENTPP_MAKE_EVENT(
    EventDisconnection, Event, (EventType::Disconnection, EventFrom::FromAny),
    G(player::Player, player)
);

EVENTPP_MAKE_EVENT(
    EventMessage, Event, (EventType::Message, EventFrom::FromAny),
    G(player::Player, player),
    G(player::Player, target),
    G(TextParse, message)
);

EVENTPP_MAKE_EVENT(
    EventPacket, Event, (EventType::Packet, EventFrom::FromAny),
    G(player::Player, player),
    G(player::Player, target),
    G(packet::GameUpdatePacket, packet),
    G(std::vector<std::byte>, ext_data)
);

struct EventPolicies {
    static EventType getEvent(const Event& e) { return e.type; }
    static bool canContinueInvoking(const Event& e) { return !e.canceled; }
};

using EventDispatcher = eventpp::HeterEventDispatcher<
    EventType,
    eventpp::HeterTuple<
        void(const EventInit&),
        void(const EventTick&),
        void(const EventConnection&),
        void(const EventDisconnection&),
        void(const EventMessage&),
        void(const EventPacket&)
    >,
    EventPolicies
>;

class Core final : public extension::Extensible {
public:
    Core();
    ~Core() override;

    bool add_extension(extension::IExtension* ext) override;

    void run();
    void stop() { run_ = false; }

    [[nodiscard]] Config& get_config() { return config_; }
    [[nodiscard]] server::Server* get_server() const { return server_; }
    [[nodiscard]] client::Client* get_client() const { return client_; }

    [[nodiscard]] EventDispatcher& get_event_dispatcher() { return event_dispatcher_; }

private:
    Config config_;

    server::Server* server_;
    client::Client* client_;

    bool run_;
    std::uint32_t tick_;

    EventDispatcher event_dispatcher_;
};
}
