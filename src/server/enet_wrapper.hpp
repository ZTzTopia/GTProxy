#pragma once
#include <cstddef>
#include <enet/enet.h>

namespace server {
class ENetWrapper {
public:
    explicit ENetWrapper(enet_uint16 port, std::size_t peer_count = ENET_PROTOCOL_MAXIMUM_PEER_ID)
        : host_{ nullptr }
    {
        ENetAddress address{};
        address.host = ENET_HOST_ANY;
        address.port = port;

        host_ = enet_host_create(&address, peer_count, 2, 0, 0);
        if (!host_) {
            return;
        }

        if (enet_host_compress_with_range_coder(host_) != 0) {
            return;
        }

        host_->checksum = enet_crc32;
        host_->usingNewPacketForServer = 1;
    }

    ~ENetWrapper() { enet_host_destroy(host_); }

    virtual void process()
    {
        if (!host_) {
            return;
        }

        ENetEvent ev{};
        while (enet_host_service(host_, &ev, 16) > 0) {
            switch (ev.type) {
            case ENET_EVENT_TYPE_CONNECT:
                on_connect(ev.peer);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                on_disconnect(ev.peer);
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                on_receive(ev.peer, ev.packet);
                break;
            default:
                break;
            }
        }
    }

    virtual void on_connect(ENetPeer* peer) = 0;
    virtual void on_receive(ENetPeer* peer, ENetPacket* packet) = 0;
    virtual void on_disconnect(ENetPeer* peer) = 0;

protected:
    ENetHost* host_;
};
}