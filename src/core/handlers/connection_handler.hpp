#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <spdlog/spdlog.h>

#include "../../event/event.hpp"
#include "../../network/client.hpp"
#include "../../network/server.hpp"
#include "../../packet/game/server.hpp"
#include "../../packet/packet_helper.hpp"
#include "../config.hpp"

namespace core {

class ConnectionHandler {
public:
    ConnectionHandler(
        event::Dispatcher& dispatcher,
        network::Client& client,
        network::Server& server,
        Config& config
    )
        : dispatcher_{ dispatcher }
        , client_{ client }
        , server_{ server }
        , config_{ config }
        , pending_port_{ 65535 }
    {
        setup_connection_handlers();
        setup_on_send_to_server_handler();
        setup_quit_handler();
        setup_disconnect_handler();
    }

private:
    void setup_connection_handlers()
    {
        handles_.emplace_back(
            dispatcher_,
            event::Type::ClientConnect,
            dispatcher_.appendListener(event::Type::ClientConnect, [this](const event::Event&) {
                if (pending_port_ == 65535) {
                    return;
                }

                spdlog::debug("Connecting to Growtopia server at {}:{}", pending_address_, pending_port_);
                client_.connect(pending_address_, pending_port_);

                pending_address_.clear();
                pending_port_ = 65535;
            })
        );

        handles_.emplace_back(
            dispatcher_,
            event::Type::ClientDisconnect,
            dispatcher_.appendListener(event::Type::ClientDisconnect, [this](const event::Event&) {
                pending_address_.clear();
                pending_port_ = 65535;

                if (!client_.is_connected()) {
                    return;
                }

                client_.disconnect();
                spdlog::info("Gracefully disconnect Growtopia server from proxy client");
            })
        );

        handles_.emplace_back(
            dispatcher_,
            event::Type::ServerDisconnect,
            dispatcher_.appendListener(event::Type::ServerDisconnect, [this](const event::Event&) {
                if (!server_.is_connected()) {
                    return;
                }

                server_.disconnect();
                spdlog::info("Gracefully disconnect Growtopia client from proxy server");
            })
        );
    }

    void setup_on_send_to_server_handler()
    {
        constexpr auto on_send_to_server_type = event::packet_event_type(packet::PacketId::OnSendToServer);
        handles_.emplace_back(
            dispatcher_,
            on_send_to_server_type,
            dispatcher_.appendListener(on_send_to_server_type, [this](const event::Event& event) {
                const auto* evt = dynamic_cast<const event::TypedPacketEvent<packet::PacketId::OnSendToServer>*>(&event);
                if (!evt) {
                    return;
                }

                const auto pkt{ evt->get<packet::game::OnSendToServer>() };
                if (!pkt) {
                    return;
                }

                pending_address_ = pkt->address;
                pending_port_ = pkt->port;

                const auto modified_pkt = std::make_shared<packet::game::OnSendToServer>(*pkt);
                modified_pkt->address = "127.0.0.1";
                modified_pkt->port = config_.get_server_config().port;

                std::ignore = packet::PacketHelper::write(*modified_pkt, server_);
                evt->cancel();
            })
        );
    }

    void setup_quit_handler()
    {
        constexpr auto quit_type{ event::packet_event_type(packet::PacketId::Quit) };
        handles_.emplace_back(
            dispatcher_,
            quit_type,
            dispatcher_.appendListener(quit_type, [this](const event::Event& event) {
                if (
                    const auto evt{ dynamic_cast<const event::TypedPacketEvent<packet::PacketId::Quit>*>(&event) };
                    !evt || evt->direction != event::Direction::ServerBound
                ) {
                    return;
                }

                server_.disconnect();
                client_.disconnect_now();
                spdlog::info("Forced disconnect proxy client from Growtopia server");
            })
        );
    }

    void setup_disconnect_handler()
    {
        constexpr auto disconnect_type{ event::packet_event_type(packet::PacketId::Disconnect) };
        handles_.emplace_back(
            dispatcher_,
            disconnect_type,
            dispatcher_.appendListener(disconnect_type, [this](const event::Event& event) {
                if (
                    const auto evt{ dynamic_cast<const event::TypedPacketEvent<packet::PacketId::Disconnect>*>(&event) };
                    !evt || evt->direction != event::Direction::ServerBound
                ) {
                    return;
                }

                server_.disconnect_now();
                spdlog::info("Forced disconnect proxy server from Growtopia client");
                client_.disconnect_now();
                spdlog::info("Forced disconnect proxy client from Growtopia server");
            })
        );
    }

private:
    event::Dispatcher& dispatcher_;
    network::Client& client_;
    network::Server& server_;
    Config& config_;

    std::string pending_address_;
    uint16_t pending_port_;

    std::vector<event::ScopedHandle> handles_;
};
}
