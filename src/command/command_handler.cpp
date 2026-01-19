#include "command_handler.hpp"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "../packet/packet_helper.hpp"
#include "../packet/message/input.hpp"
#include "commands/exit_command.hpp"
#include "commands/proxy_command.hpp"
#include "commands/warp_command.hpp"

namespace command {
namespace {
    constexpr auto input_event_type = event::packet_event_type(packet::PacketId::Input);
}

CommandHandler::CommandHandler(
    core::Config& config,
    event::Dispatcher& dispatcher,
    std::shared_ptr<core::Scheduler> scheduler,
    network::Server& server,
    network::Client& client
)
    : config_{ config }
    , dispatcher_{ dispatcher }
    , scheduler_{ std::move(scheduler) }
    , server_{ server }
    , client_{ client }
{
    registry_.set_prefix(config_.get_command_config().prefix);
    register_default_commands();

    listener_handle_ = dispatcher_.prependListener(
        input_event_type,
        [this](const event::Event& e) { on_text_packet(e); }
    );

    spdlog::info("Command handler initialized with prefix '{}'", registry_.prefix());
}

CommandHandler::~CommandHandler()
{
    dispatcher_.removeListener(input_event_type, listener_handle_);
}

void CommandHandler::register_default_commands()
{
    registry_.add(std::make_unique<ProxyCommand>());
    registry_.add(std::make_unique<WarpCommand>());
    registry_.add(std::make_unique<ExitCommand>());
}

void CommandHandler::on_text_packet(const event::Event& e)
{
    const auto* evt = dynamic_cast<const event::TypedPacketEvent<packet::PacketId::Input>*>(&e);
    if (!evt) {
        return;
    }

    const auto input_pkt{ evt->get<packet::message::Input>() };
    if (!input_pkt) {
        return;
    }

    const std::string& text = input_pkt->text;
    if (registry_.execute(text, server_, client_, dispatcher_, scheduler_)) {
        spdlog::info("Command handler executed successfully");
        evt->cancel();
    }
}
}
