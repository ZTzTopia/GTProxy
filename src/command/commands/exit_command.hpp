#pragma once
#include "../command.hpp"
#include "../../packet/packet_helper.hpp"
#include "../../packet/message/exit.hpp"

namespace command {
class ExitCommand final : public ICommand {
public:
    [[nodiscard]] std::string_view name() const override { return "exit"; }
    [[nodiscard]] std::string description() const override { return "Exit the current world."; }

    Result execute(const Context& ctx) override
    {
        packet::message::QuitToExit pkt{};
        packet::PacketHelper::write(pkt, ctx.client);
        return Result::Success;
    }
};
}
