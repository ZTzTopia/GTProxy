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

        std::vector variants{ variant.get_variants() };
        if (variants.empty()) {
            spdlog::warn("Variants are empty");
            return;
        }

        if (core_->get_config().get<bool>("log.printVariant"))  {
            spdlog::info("Incoming variant from {}:", event.from == core::EventFrom::FromClient ? "client" : "server");
            for (const auto& var : variants) {
                switch (packet::Variant::get_type(var)) {
                case packet::VariantType::FLOAT:
                    spdlog::info("\t[FLOAT]: {}", std::get<float>(var));
                    break;
                case packet::VariantType::STRING: {
                    TextParse text_parse{ std::get<std::string>(var) };
                    if (!text_parse.empty()) {
                        std::vector key_values{ text_parse.get_key_values() };
                        if (key_values.size() == 1) {
                            spdlog::info("\t[STRING]: {}", key_values[0]);
                            break;
                        }

                        spdlog::info("\t[STRING]:");
                        for (const auto& key_value : text_parse.get_key_values()) {
                            spdlog::info("\t\t{}", key_value);
                        }

                        break;
                    }

                    spdlog::info("\t[STRING]: {}", std::get<std::string>(var));
                    break;
                }
                case packet::VariantType::VEC2: {
                    const glm::vec2 vec2{ std::get<glm::vec2>(var) };
                    spdlog::info("\t[VEC2]: x: {}, y: {}", vec2.x, vec2.y);
                    break;
                }
                case packet::VariantType::UNSIGNED:
                    spdlog::info("\t[UNSIGNED]: {}", std::get<uint32_t>(var));
                    break;
                case packet::VariantType::SIGNED:
                    spdlog::info("\t[SIGNED]: {}", std::get<int32_t>(var));
                    break;
                default:
                    break;
                }
            }
        }

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
