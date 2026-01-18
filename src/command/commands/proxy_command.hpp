#pragma once
#include "../command.hpp"
#include "../../packet/packet_helper.hpp"
#include "../../packet/message/chat.hpp"

namespace command {
class ProxyCommand final : public ICommand {
public:
    [[nodiscard]] std::string_view name() const override { return "proxy"; }

    Result execute(const Context& ctx) override
    {
        packet::message::Log pkt{};
        pkt.msg = "Proxy command executed successfully!";
        packet::PacketHelper::write(pkt, ctx.server);
        return Result::Success;
    }
};
}
