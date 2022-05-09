#pragma once
#include <thread>
#include <atomic>
#include <enet/enet.h>

namespace enetwrapper {
    class ENetServer {
    public:
        ENetServer();
        ~ENetServer();

        bool create_host(enet_uint16 port, std::size_t peer_count);
        void destroy_host();

        void start_service();
        void service_thread();

        virtual void on_connect(ENetPeer* peer) = 0;
        virtual void on_receive(ENetPeer* peer, ENetPacket* packet) = 0;
        virtual void on_disconnect(ENetPeer* peer) = 0;

    protected:
        ENetHost* m_host;
        std::thread m_service_thread;
        std::atomic<bool> m_running;
    };
}