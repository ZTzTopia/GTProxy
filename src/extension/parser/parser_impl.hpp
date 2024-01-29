#pragma once
#include "parser.hpp"
#include "../../client/client.hpp"
#include "../../core/core.hpp"
#include "../../server/server.hpp"

namespace extension::parser {
class ParserExtension final : public IParserExtension {
    core::Core* core_;

    MessageCallback message_callback_;

public:
    explicit ParserExtension(core::Core* core)
        : core_{ core }
    {

    }

    ~ParserExtension() override = default;

    void init() override
    {
        core_->get_server()->get_message_callback().prepend([this](
            const player::Player& from, const player::Player& to, const std::string& message
        ) {
            message_callback_(ParserCallback{
                ParseType::FromClient,
                from,
                to,
                TextParse{ message }
            });
            return true;
        });

        core_->get_client()->get_message_callback().prepend([this](
            const player::Player& from, const player::Player& to, const std::string& message
        ) {
            message_callback_(ParserCallback{
                ParseType::FromServer,
                from,
                to,
                TextParse{ message }
            });
            return true;
        });
    }

    void free() override
    {
        delete this;
    }

    MessageCallback& get_message_callback() override
    {
        return message_callback_;
    }
};
}
