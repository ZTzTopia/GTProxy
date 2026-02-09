#include "forwarding_handler.hpp"

namespace core::handlers {
ForwardingHandler::ForwardingHandler(
    event::Dispatcher& dispatcher,
    network::Client& client,
    network::Server& server
)
    : dispatcher_{ dispatcher }
    , client_{ client }
    , server_{ server }
{
    setup_raw_packet_handlers();
}

void ForwardingHandler::setup_raw_packet_handlers()
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
}
