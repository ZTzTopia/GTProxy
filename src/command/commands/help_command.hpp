#pragma once
#include <string>
#include <vector>
#include <fmt/format.h>

#include "../command.hpp"
#include "../command_registry.hpp"
#include "../../packet/packet_helper.hpp"
#include "../../packet/message/chat.hpp"

namespace command {
class HelpCommand final : public ICommand {
public:
    [[nodiscard]] std::string_view name() const override { return "proxyhelp"; }
    [[nodiscard]] std::string description() const override { return "List all commands or show command usage"; }

    Result execute(const Context& ctx) override
    {
        if (ctx.args.empty()) {
            auto commands{ ctx.registry.get_all_commands() };
            
            packet::message::Log log_header{};
            log_header.msg = fmt::format("Available commands: {}", commands.size());
            packet::PacketHelper::write(log_header, ctx.server);

            for (const auto& [name, desc] : commands) {
                packet::message::Log log_item{};
                log_item.msg = fmt::format("{}{}: ``{}", ctx.registry.prefix(), name, desc);
                packet::PacketHelper::write(log_item, ctx.server);
            }

            return Result::Success;
        }

        std::string cmd_name{ ctx.args[0] };
        if (auto* cmd = ctx.registry.get(cmd_name)) {
            packet::message::Log log_info{};
            log_info.msg = fmt::format("{}{}: ``{}", ctx.registry.prefix(), cmd_name, cmd->description());
            packet::PacketHelper::write(log_info, ctx.server);
            return Result::Success;
        }

        packet::message::Log log_error{};
        log_error.msg = fmt::format("`4Error: ``Command '{}' not found", cmd_name);
        packet::PacketHelper::write(log_error, ctx.server);
        return Result::Failed;
    }
};
}
