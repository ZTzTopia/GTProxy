#pragma once
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
            [this](const core::EventPacket& event)
            {
                switch (event.get_packet().type) {
                case packet::PacketType::PACKET_CALL_FUNCTION:
                    parse_call_function(event);
                    break;
                default:
                    break;
                }
            }
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

private:
    void parse_call_function(const core::EventPacket& event) const
    {
        if (event.from == core::EventFrom::FromClient) {
            return;
        }

        packet::Variant variant{};
        if (!variant.deserialize(event.get_ext_data())) {
            spdlog::warn("Failed to deserialize variant");
            return;
        }

        // TODO: Print the variant
        // spdlog::info("Variant: {}", variant);

        const EventCallFunction event_call_function{
            event.get_player(),
            event.get_target(),
            variant.get(0),
            variant
        };
        event_call_function.from = event.from;

        event_dispatcher_.dispatch(event_call_function);
        event.canceled = event_call_function.canceled;
    }
};
}
