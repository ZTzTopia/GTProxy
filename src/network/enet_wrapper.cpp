#include "enet_wrapper.hpp"

namespace network {
ENetWrapper::ENetWrapper(ENetHost* host)
    : host_{host}
{ }

ENetWrapper::~ENetWrapper()
{
    if (!host_) {
        return;
    }

    enet_host_destroy(host_);
    host_ = nullptr;
}

void ENetWrapper::process()
{
    if (!host_) {
        return;
    }

    ENetEvent event{};
    while (enet_host_service(host_, &event, 0) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            on_connect(event.peer);
            break;
        case ENET_EVENT_TYPE_RECEIVE: {
            on_receive(
                event.peer,
                std::span{
                    reinterpret_cast<std::byte*>(event.packet->data),
                    event.packet->dataLength
                }
            );
            enet_packet_destroy(event.packet);
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
            on_disconnect(event.peer);
            break;
        default:
            break;
        }
    }
}
}
