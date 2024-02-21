#pragma once
#include "../extension.hpp"
#include "../../packet/packet_variant.hpp"
#include "../../player/player.hpp"

struct IParserExtension : extension::IExtension {
    PROVIDE_EXT_UID(0x4ea75473);

    enum class EventType {
        CallFunction
    };

    class Event
    {
    public:
        explicit Event(const EventType type)
            : type{ type }
            , from{ core::EventFrom::FromAny }
            , canceled{ false }
        {

        }

        Event(const EventType type, const core::EventFrom from)
            : type{ type }
            , from{ from }
            , canceled{ false }
        {

        }

        EventType type;
        mutable core::EventFrom from;
        mutable bool canceled;
    };

    EVENTPP_MAKE_EVENT(
        EventCallFunction, Event, (EventType::CallFunction, core::EventFrom::FromAny),
        G(player::Player, player),
        G(player::Player, target),
        G(std::string, function_name),
        G(packet::Variant, args)
    );

    struct EventPolicies {
        using ArgumentPassingMode = eventpp::ArgumentPassingIncludeEvent;

        static EventType getEvent(const Event& e) { return e.type; }
        static bool canContinueInvoking(const Event& e) { return !e.canceled; }
    };

    using EventDispatcher = eventpp::HeterEventDispatcher<
        EventType,
        eventpp::HeterTuple<
            void(const EventCallFunction&)
        >,
        EventPolicies
    >;
    [[nodiscard]] virtual EventDispatcher& get_event_dispatcher() = 0;
};
