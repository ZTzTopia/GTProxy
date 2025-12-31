#include "session_handler.hpp"

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
    dispatcher_.appendListener(event::Type::ClientBoundPacket, [this](const event::Event& event) {
        const auto& raw_packet{ dynamic_cast<const event::RawPacketEvent*>(&event) };
        if (!raw_packet) {
            return;
        }

        std::ignore = server_.write(raw_packet->data);
    });

    dispatcher_.appendListener(event::Type::ServerBoundPacket, [this](const event::Event& event) {
        const auto& raw_packet{ dynamic_cast<const event::RawPacketEvent*>(&event) };
        if (!raw_packet) {
            return;
        }

        std::ignore = client_.write(raw_packet->data);
    });

    dispatcher_.appendListener(event::Type::ClientBoundPacket, [this](const event::Event& event) {
        const auto& packet{ dynamic_cast<const event::PacketEvent<packet::IPacket>*>(&event) };
        if (!packet) {
            return;
        }

        std::ignore = packet::PacketHelper::write(*(packet->packet), server_);
    });

    dispatcher_.appendListener(event::Type::ServerBoundPacket, [this](const event::Event& event) {
        const auto& packet{ dynamic_cast<const event::PacketEvent<packet::IPacket>*>(&event) };
        if (!packet) {
            return;
        }

        std::ignore = packet::PacketHelper::write(*(packet->packet), client_);
    });

    dispatcher_.prependListener(event::Type::ServerBoundPacket, [this](const event::Event& event) {
        if (const auto& packet{ dynamic_cast<const event::PacketEvent<packet::message::Quit>*>(&event) }; !packet) {
            return;
        }

        server_.disconnect(); // Umm, I think the Growtopia Server use disconnect now after sending this packet
        client_.disconnect_now(); // Use peer reset instead because the Growtopia Server is use disconnect now after
        // sending this packet
        spdlog::info("Forced disconnect proxy client from Growtopia server");
    });

    dispatcher_.prependListener(event::Type::ClientBoundPacket, [this](const event::Event& event) {
        spdlog::debug("Intercepted OnSendToServer packet, redirecting connection...");
        const auto& packet{ dynamic_cast<const event::PacketEvent<packet::game::OnSendToServer>*>(&event) };
        if (!packet) {
            spdlog::debug("Not an OnSendToServer packet, ignoring...");
            return;
        }

        packet::game::OnSendToServer pkt{ *(packet->packet) };
        pending_address_ = pkt.address;
        pending_port_ = pkt.port;

        event.cancel();

        pkt.address = "127.0.0.1";
        pkt.port = config_.get_server_config().port;

        std::ignore = packet::PacketHelper::write(pkt, server_);
    });

    dispatcher_.appendListener(event::Type::ServerBoundPacket, [this](const event::Event& event) {
        if (const auto& packet{ dynamic_cast<const event::PacketEvent<packet::game::Disconnect>*>(&event) }; !packet) {
            return;
        }

        server_.disconnect_now();
        spdlog::info("Forced disconnect proxy server from Growtopia client");
        client_.disconnect_now(); // Use peer reset instead because the Growtopia Server is use disconnect now after
        // receiving this packet
        spdlog::info("Forced disconnect proxy client from Growtopia server");
    });

    dispatcher_.prependListener(event::Type::ClientConnect, [this](const event::Event& e) {
        if (pending_port_ == 65535) {
           return;
        }

        spdlog::debug("Connecting to Growtopia server at {}:{}", pending_address_, pending_port_);

        client_.connect(pending_address_, pending_port_);

        e.cancel();

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

        // If the server timeout happens, the client will gracefully disconnect and not log the connection timeout message.
        // Can we make the client log the connection timeout message?
        server_.disconnect();
        spdlog::info("Gracefully disconnect Growtopia client from proxy server");
    });
}

SessionHandler::~SessionHandler()
{

}
}
