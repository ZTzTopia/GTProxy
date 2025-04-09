#pragma once
#include "command_handler.hpp"
#include "../parser/parser.hpp"
#include "../../core/core.hpp"
#include "../../client/client.hpp"
#include "../../packet/game/core.hpp"
#include "../../utils/text_parse.hpp"
#include "../../core/logger.hpp"
#include "../../utils/packet_utils.hpp"
#include "../../server/server.hpp"

namespace extension::command_handler {
class CommandHandlerExtension final : public ICommandHandlerExtension {
    core::Core* core_;

    std::string address_;
    int port_;

public:
    explicit CommandHandlerExtension(core::Core* core): core_{ core }{

    }

    ~CommandHandlerExtension() override = default;

    void init() override {
        core_->get_event_dispatcher().prependListener(
            core::EventType::Message,
            [this](const core::EventMessage& event) {
                // First, re-tokenize the message by retrieving its raw string.
                TextParse textParse(event.get_message().get_raw(), "|");
                std::vector<std::string> key_values = textParse.get_key_values("|");
                
                // Extract the command from the key "text"
                std::string command = textParse.get("text");
                
                // Check if the command is "/proxy" and log additional information.
                if (command == "/proxy") {
                    spdlog::info("Command /proxy executed: sending a message packet...");
                
                    player::Player* to_player = core_->get_server()->get_player();
                    if (to_player) {
                        utils::PacketUtils::send_chat_message(to_player, "Proxy command executed successfully!");
                    } else {
                        spdlog::error("No player found to send the message packet.");
                    }
                
                    event.canceled = true;
                }
            }
        );
    }

    void free() override
    {
        delete this;
    }   
};
}