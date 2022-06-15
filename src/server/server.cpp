#include <spdlog/spdlog.h>

#include "server.h"
#include "../client/client.h"
#include "../utils/text_parse.h"

namespace server {
    Server::Server(Config* config) : enetwrapper::ENetServer{}, m_config{ config }, m_client{ nullptr }, m_peer{ nullptr }
    {
        m_http = new Http{ config };
        m_http->listen("0.0.0.0", 443);
    }

    Server::~Server()
    {
        delete m_http;
        delete m_peer;
        delete m_client;
    }

    bool Server::start()
    {
        if (!create_host(17000, 1))
            return false;

        start_service();
        return true;
    }

    void Server::on_connect(ENetPeer* peer)
    {
        spdlog::info("New client connected to proxy server!");

        // No need to check if the client is disconnected, because we
        // only accept one peer at a time.

        m_peer = new player::Peer{ peer };

        // Request server data using saved headers and params from the growtopia client.
        utils::TextParse text_parse{ m_http->request_server_data() }; // TODO: Handle crash.
        std::string host = text_parse.get("server", 1);
        auto port = text_parse.get<enet_uint16>("port", 1);

        m_client = new client::Client{ m_config, this };
        if (!m_client->start(host, port)) {
            spdlog::error("Failed to connect to client!");

            delete m_client;
            m_client = nullptr;
        }
    }

    void Server::on_receive(ENetPeer* peer, ENetPacket* packet)
    {
        if (!m_client)
            return;

        if (!m_client->get_peer())
            return;

        if (!m_client->get_peer()->is_connected())
            return;

        /*if (!process_packet(peer, packet))
            return;

        m_client->get_peer()->send_packet_packet(packet);*/
    }

    void Server::on_disconnect(ENetPeer* peer)
    {
        spdlog::info("Client disconnected from proxy server!");

        // The client will disconnect now from growtopia server after
        // deleting the client object.
        delete m_client;
        m_client = nullptr;

        // Yes, useless log.
        spdlog::info("Disconnected from growtopia server!");

        delete m_peer;
        m_peer = nullptr;
    }

    bool Server::process_packet(ENetPeer* peer, ENetPacket* packet)
    {
        return true;
    }
}
