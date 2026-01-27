#pragma once
#include <vector>

#include "../../event/event.hpp"
#include "../../network/client.hpp"
#include "../../network/server.hpp"
#include "../../packet/packet_helper.hpp"

namespace core {

class ForwardingHandler {
public:
    ForwardingHandler(
        event::Dispatcher& dispatcher,
        network::Client& client,
        network::Server& server
    )
        : dispatcher_{ dispatcher }
        , client_{ client }
        , server_{ server }
    {
        setup_raw_packet_handlers();
        setup_unknown_packet_forwarders();
    }

private:
    void setup_raw_packet_handlers()
    {
        handles_.emplace_back(
            dispatcher_,
            event::Type::ClientBoundPacket,
            dispatcher_.appendListener(event::Type::ClientBoundPacket, [this](const event::Event& event) {
                const auto raw_packet{ dynamic_cast<const event::RawPacketEvent*>(&event) };
                if (!raw_packet) {
                    return;
                }

                std::ignore = server_.write(raw_packet->data);
            })
        );

        handles_.emplace_back(
            dispatcher_,
            event::Type::ServerBoundPacket,
            dispatcher_.appendListener(event::Type::ServerBoundPacket, [this](const event::Event& event) {
                const auto raw_packet{ dynamic_cast<const event::RawPacketEvent*>(&event) };
                if (!raw_packet) {
                    return;
                }

                std::ignore = client_.write(raw_packet->data);
            })
        );
    }

    void setup_unknown_packet_forwarders()
    {
        handles_.emplace_back(
            dispatcher_,
            event::Type::ClientBoundPacket,
            dispatcher_.appendListener(event::Type::ClientBoundPacket, [this](const event::Event& event) {
                const auto pkt_event{ dynamic_cast<const event::PacketEvent*>(&event) };
                if (!pkt_event || !pkt_event->has_packet()) {
                    return;
                }

                std::ignore = packet::PacketHelper::write(*(pkt_event->packet), server_);
            })
        );

        handles_.emplace_back(
            dispatcher_,
            event::Type::ServerBoundPacket,
            dispatcher_.appendListener(event::Type::ServerBoundPacket, [this](const event::Event& event) {
                const auto pkt_event{ dynamic_cast<const event::PacketEvent*>(&event) };
                if (!pkt_event || !pkt_event->has_packet()) {
                    return;
                }

                std::ignore = packet::PacketHelper::write(*(pkt_event->packet), client_);
            })
        );
    }

private:
    event::Dispatcher& dispatcher_;
    network::Client& client_;
    network::Server& server_;

    std::vector<event::ScopedHandle> handles_;
};

}
