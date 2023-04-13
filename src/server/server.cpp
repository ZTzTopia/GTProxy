#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "server.h"
#include "../client/client.h"
#include "../utils/hash.h"
#include "../utils/text_parse.h"

namespace server {
Server::Server(Config* config)
    : enet_wrapper::ENetServer{}
    , m_config{ config }
    , m_peer{ nullptr }
{
    m_http = new Http{ config };
    m_client = new client::Client{ m_config, m_http, this };
}

Server::~Server()
{
    delete m_http;
    delete m_peer.m_gt_client;
    delete m_client;
}

bool Server::start()
{
    if (!m_http->listen("0.0.0.0", 443)) {
        return false;
    }

    if (!create_host(m_config->get_host().m_port, 1, 1)) {
        spdlog::error("Failed to create ENet server host.");
        return false;
    }

    start_service();
    spdlog::info("ENet server listening on port {}.", m_config->get_host().m_port);
    return true;
}

void Server::on_connect(ENetPeer* peer)
{
    spdlog::info("New client connected to proxy server.");

    m_peer.m_gt_client = new player::Peer{ peer };
    m_client->set_gt_client_peer(m_peer.m_gt_client);
    m_client->start();
}

void Server::on_receive(ENetPeer* peer, ENetPacket* packet)
{
    if (!m_peer.m_gt_server) {
        return;
    }

    if (!m_peer.m_gt_server->is_connected()) {
        return;
    }

    if (!process_packet(peer, packet)) {
        return;
    }

    m_peer.m_gt_server->send_packet_packet(packet);
}

void Server::on_disconnect(ENetPeer* peer)
{
    spdlog::info("Client disconnected from proxy server.");

    if (m_peer.m_gt_server && m_peer.m_gt_server->is_connected()) {
        m_peer.m_gt_server->disconnect();
    }

    delete m_peer.m_gt_client;
    m_peer.m_gt_client = nullptr;
    m_client->set_gt_client_peer(nullptr);
}

bool Server::process_packet(ENetPeer* peer, ENetPacket* packet)
{
    player::eNetMessageType message_type{ player::message_type_to_string(packet) };
    std::string message_data{ player::get_text(packet) };

    if (message_type != player::NET_MESSAGE_GAME_PACKET) {
        utils::TextParse text_parse{ message_data };
        if (!text_parse.empty()) {
            spdlog::info(
                "Outgoing MessagePacket:\n{} [{}]:\n{}\n",
                magic_enum::enum_name(message_type),
                message_type,
                fmt::join(text_parse.get_all_array(), "\r\n")
            );
        }
    }

    switch (message_type) {
        case player::NET_MESSAGE_GENERIC_TEXT: {
            if (message_data.find("action|input") != std::string::npos) {
                utils::TextParse text_parse{ message_data };
                if (text_parse.get("text", 1).empty()) {
                    break;
                }

                std::string str{ message_data.substr(
                    message_data.find("text|") + 5,
                    message_data.length() - message_data.find("text|") - 1
                ) };

                if (str.substr(0, 6) == m_config->get_command().m_prefix + "warp ") {
                    std::string world{ str.substr(6, message_data.length() - 6 - 1) };
                    m_peer.m_gt_server->send_packet(
                        player::eNetMessageType::NET_MESSAGE_GAME_MESSAGE,
                        fmt::format("action|join_request\nname|{}\ninvitedWorld|0", world)
                    );
                    return false;
                }
            }

            break;
        }
        case player::NET_MESSAGE_GAME_PACKET: {
            player::GameUpdatePacket* game_update_packet{ player::get_struct(packet) };
            return process_tank_update_packet(peer, game_update_packet);
        }
        default:
            break;
    }

    return true;
}

bool Server::process_tank_update_packet(ENetPeer* peer, player::GameUpdatePacket* game_update_packet)
{
    switch (game_update_packet->type) {
        case player::PACKET_DISCONNECT:
            m_peer.m_gt_client->disconnect_now();
            break;
        default:
            break;
    }

    return true;
}
}
