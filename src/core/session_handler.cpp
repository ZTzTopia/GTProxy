#include "session_handler.hpp"

#include "../packet/packet_event_registry.hpp"
#include "../packet/packet_helper.hpp"
#include "../packet/game/server.hpp"
#include "../packet/game/world.hpp"
#include "../world/world.hpp"

namespace core {
SessionHandler::SessionHandler(
    Config& config,
    event::Dispatcher& dispatcher,
    network::Client& client,
    network::Server& server
)
    : config_{ config }
    , dispatcher_{ dispatcher }
    , client_{ client }
    , server_{ server }
    , pending_port_{ 65535 }
{
    setup_raw_packet_handlers();
    setup_unknown_packet_forwarders();
    setup_disconnect_handler();
    setup_connection_handlers();
    setup_on_send_to_server_handler();
    setup_quit_handler();
    setup_join_request_handler();
    setup_on_spawn_handler();
    setup_on_remove_handler();
}

void SessionHandler::setup_raw_packet_handlers() const
{
    dispatcher_.appendListener(event::Type::ClientBoundPacket, [this](const event::Event& event) {
        const auto* raw_packet = dynamic_cast<const event::RawPacketEvent*>(&event);
        if (!raw_packet) {
            return;
        }

        std::ignore = server_.write(raw_packet->data);
    });

    dispatcher_.appendListener(event::Type::ServerBoundPacket, [this](const event::Event& event) {
        const auto* raw_packet = dynamic_cast<const event::RawPacketEvent*>(&event);
        if (!raw_packet) {
            return;
        }

        std::ignore = client_.write(raw_packet->data);
    });
}

void SessionHandler::setup_unknown_packet_forwarders() const
{
    dispatcher_.appendListener(event::Type::ClientBoundPacket, [this](const event::Event& event) {
        const auto* pkt_event = dynamic_cast<const event::PacketEvent*>(&event);
        if (!pkt_event || !pkt_event->has_packet()) {
            return;
        }

        std::ignore = packet::PacketHelper::write(*(pkt_event->packet), server_);
    });

    dispatcher_.appendListener(event::Type::ServerBoundPacket, [this](const event::Event& event) {
        const auto* pkt_event = dynamic_cast<const event::PacketEvent*>(&event);
        if (!pkt_event || !pkt_event->has_packet()) {
            return;
        }

        std::ignore = packet::PacketHelper::write(*(pkt_event->packet), client_);
    });
}

void SessionHandler::setup_on_send_to_server_handler()
{
    constexpr auto on_send_to_server_type = event::packet_event_type(packet::PacketId::OnSendToServer);
    dispatcher_.appendListener(on_send_to_server_type, [this](const event::Event& event) {
        const auto* evt = dynamic_cast<const event::TypedPacketEvent<packet::PacketId::OnSendToServer>*>(&event);
        if (!evt) {
            return;
        }

        auto pkt = evt->template get<packet::game::OnSendToServer>();
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
    });
}

void SessionHandler::setup_quit_handler() const
{
    constexpr auto quit_type = event::packet_event_type(packet::PacketId::Quit);
    dispatcher_.appendListener(quit_type, [this](const event::Event& event) {
        const auto* evt = dynamic_cast<const event::TypedPacketEvent<packet::PacketId::Quit>*>(&event);
        if (!evt || evt->direction != event::Direction::ServerBound) {
            return;
        }

        server_.disconnect();
        client_.disconnect_now();
        spdlog::info("Forced disconnect proxy client from Growtopia server");
    });
}

void SessionHandler::setup_join_request_handler() const
{
    constexpr auto join_request_type = event::packet_event_type(packet::PacketId::JoinRequest);
    dispatcher_.appendListener(join_request_type, [](const event::Event& event) {
        const auto* evt = dynamic_cast<const event::TypedPacketEvent<packet::PacketId::JoinRequest>*>(&event);
        if (!evt || evt->direction != event::Direction::ServerBound) {
            return;
        }

        world::World::instance().clear();
    });
}

void SessionHandler::setup_disconnect_handler() const
{
    constexpr auto disconnect_type = event::packet_event_type(packet::PacketId::Disconnect);
    dispatcher_.appendListener(disconnect_type, [this](const event::Event& event) {
        const auto* evt = dynamic_cast<const event::TypedPacketEvent<packet::PacketId::Disconnect>*>(&event);
        if (!evt || evt->direction != event::Direction::ServerBound) {
            return;
        }

        server_.disconnect_now();
        spdlog::info("Forced disconnect proxy server from Growtopia client");
        client_.disconnect_now();
        spdlog::info("Forced disconnect proxy client from Growtopia server");
    });
}

void SessionHandler::setup_connection_handlers()
{
    dispatcher_.appendListener(event::Type::ClientConnect, [this](const event::Event& e) {
        if (pending_port_ == 65535) {
           return;
        }

        spdlog::debug("Connecting to Growtopia server at {}:{}", pending_address_, pending_port_);

        client_.connect(pending_address_, pending_port_);

        pending_address_.clear();
        pending_port_ = 65535;
    });

    dispatcher_.appendListener(event::Type::ClientDisconnect, [this](const event::Event& e) {
        pending_address_.clear();
        pending_port_ = 65535;

        if (!client_.is_connected()) {
           return;
       }

       client_.disconnect();
       spdlog::info("Gracefully disconnect Growtopia server from proxy client");
    });

    dispatcher_.appendListener(event::Type::ServerDisconnect, [this](const event::Event& e) {
        if (!server_.is_connected()) {
            return;
        }

        server_.disconnect();
        spdlog::info("Gracefully disconnect Growtopia client from proxy server");
    });
}

void SessionHandler::setup_on_spawn_handler() const
{
    constexpr auto on_spawn_type = event::packet_event_type(packet::PacketId::OnSpawn);
    dispatcher_.appendListener(on_spawn_type, [](const event::Event& event) {
        const auto* evt = dynamic_cast<const event::TypedPacketEvent<packet::PacketId::OnSpawn>*>(&event);
        if (!evt || evt->direction != event::Direction::ClientBound) {
            return;
        }

        auto pkt = evt->template get<packet::game::OnSpawn>();
        if (!pkt) {
            return;
        }

        const auto player{ player::Player::from_on_spawn(*pkt) };
        world::World::instance().add_player(player);
    });
}

void SessionHandler::setup_on_remove_handler() const
{
    constexpr auto on_remove_type = event::packet_event_type(packet::PacketId::OnRemove);
    dispatcher_.appendListener(on_remove_type, [](const event::Event& event) {
        const auto* evt = dynamic_cast<const event::TypedPacketEvent<packet::PacketId::OnRemove>*>(&event);
        if (!evt || evt->direction != event::Direction::ClientBound) {
            return;
        }

        auto pkt = evt->template get<packet::game::OnRemove>();
        if (!pkt) {
            return;
        }

        world::World::instance().remove_player(pkt->net_id);
    });
}

SessionHandler::~SessionHandler()
{

}
}
