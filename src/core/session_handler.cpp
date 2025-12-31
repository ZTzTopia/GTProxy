#include "session_handler.hpp"

namespace core {
SessionHandler::SessionHandler(
    Config& config,
    event::Dispatcher& dispatcher,
    network::Client& client,
    network::Server& server
)
    : config{ config }
    , dispatcher_{ dispatcher }
    , client_{ client }
    , server_{ server }
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

    dispatcher_.appendListener(event::Type::ClientBoundPacket, [this](const event::Event& event) {
        if (const auto& raw_packet{ dynamic_cast<const event::PacketEvent<packet::message::Quit>*>(&event) }; !raw_packet) {
            return;
        }

        server_.disconnect(); // Umm, I think the Growtopia Server use disconnect now after sending this packet
        client_.disconnect_now(); // Use peer reset instead because the Growtopia Server is use disconnect now after
        // sending this packet
        spdlog::info("Forced disconnect proxy client from server");
    });

    dispatcher_.appendListener(event::Type::ClientBoundPacket, [this](const event::Event& event) {
        if (const auto& raw_packet{ dynamic_cast<const event::PacketEvent<packet::game::Disconnect>*>(&event) }; !raw_packet) {
            return;
        }

        server_.disconnect_now();
        spdlog::info("Forced disconnect proxy server from client");
        client_.disconnect_now(); // Use peer reset instead because the Growtopia Server is use disconnect now after
        // receiving this packet
        spdlog::info("Forced disconnect proxy client from server");
    });

    dispatcher_.appendListener(event::Type::ClientDisconnect, [this](const event::Event& e) {
        if (!client_.is_connected()) {
           return;
       }

       client_.disconnect();
       spdlog::info("Gracefully disconnect server from proxy client");
    });

    dispatcher_.appendListener(event::Type::ServerDisconnect, [this](const event::Event& e) {
        if (!server_.is_connected()) {
            return;
        }

        // If the server timeout happens, the client will gracefully disconnect and not log the connection timeout message.
        // Can we make the client log the connection timeout message?
        server_.disconnect();
        spdlog::info("Gracefully disconnect client from proxy server");
    });
}

SessionHandler::~SessionHandler()
{

}
}
