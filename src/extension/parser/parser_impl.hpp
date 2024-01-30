#pragma once
#include <eventpp/utilities/argumentadapter.h>

#include "parser.hpp"
#include "../../core/core.hpp"

namespace extension::parser {
class ParserExtension final : public IParserExtension {
    core::Core* core_;

    EventDispatcher event_dispatcher_;

public:
    explicit ParserExtension(core::Core* core)
        : core_{ core }
    {

    }

    ~ParserExtension() override = default;

    void init() override
    {
        core_->get_event_dispatcher().prependListener(
            core::EventType::Packet,
            eventpp::argumentAdapter<void(const core::EventPacket&)>([this](
                const core::EventPacket& event
            ) {
                if (event.get_packet().type != packet::PacketType::PACKET_CALL_FUNCTION) {
                    return;
                }

                packet::Variant variant{};
                if (!variant.deserialize(event.get_ext_data())) {
                    spdlog::warn("Failed to deserialize variant");
                    return;
                }

                const EventCallFunction event_call_function{
                    event.get_player(),
                    event.get_target(),
                    variant.get(0),
                    variant
                };
                event_call_function.from = event.from;
                event_dispatcher_.dispatch(event_call_function);
            })
        );
    }

    void free() override
    {
        delete this;
    }

    EventDispatcher& get_event_dispatcher() override
    {
        return event_dispatcher_;
    }
};
}
