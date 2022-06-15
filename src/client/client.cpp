#include "client.h"
#include "../server/server.h"

namespace client {
    Client::Client(Config* config, server::Server* server) : enetwrapper::ENetClient{}, m_config{ config }, m_peer{ nullptr }, m_server{ server } {}

    Client::~Client()
    {
        delete m_peer;
    }

    bool Client::start(const std::string& host, enet_uint16 port)
    {
        if (!create_host(1, m_config->m_server.using_new_packet))
            return false;

        if (!connect(host, port))
            return false;

        start_service();
        return true;
    }

    void Client::on_connect(ENetPeer* peer)
    {
        spdlog::info("Connected to growtopia server!");

        m_peer = new player::Peer{ peer };
    }

    void Client::on_receive(ENetPeer* peer, ENetPacket* packet)
    {
        if (!m_server->get_peer())
            return;

        if (!m_server->get_peer()->is_connected())
            return;

        /*if (!process_packet(peer, packet))
            return;

        m_server->get_peer()->send_packet_packet(packet);*/
    }

    void Client::on_disconnect(ENetPeer* peer)
    {
        spdlog::info("Disconnected from growtopia server!");

        // Don't use disconnect now because it will not send disconnect event.
        if (m_server->get_peer()->is_connected()) {
            m_server->get_peer()->disconnect();
        }

        delete m_peer;
        m_peer = nullptr;
    }

    bool Client::process_packet(ENetPeer* peer, ENetPacket* packet)
    {
        return true;
    }
}
