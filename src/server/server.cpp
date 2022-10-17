#include <magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>

#include "server.h"
#include "../client/client.h"
#include "../enetwrapper/enet_wrapper.h"
#include "../utils/random.h"
#include "../utils/text_parse.h"

namespace server {
Server::Server(Config* config)
    : enetwrapper::ENetServer{}, m_config{ config }, m_client{ nullptr }, m_peer{ nullptr }
{
    m_http = new Http{ config };
}

Server::~Server()
{
    delete m_http;
    delete m_peer;
    delete m_client;
}

bool Server::start()
{
    if (!m_http->listen("0.0.0.0", 443)) {
        return false;
    }

    if (!enetwrapper::ENetWrapper::one_time_init()) {
        spdlog::error("Failed to initialize ENet server.");
        return false;
    }

    if (!create_host(m_config->m_host.port, 1, 1)) {
        spdlog::error("Failed to create ENet server host.");
        return false;
    }

    start_service();

    spdlog::info("ENet server listening on port {}.", m_config->m_host.port);
    return true;
}

void Server::on_connect(ENetPeer* peer)
{
    spdlog::info("New client connected to proxy server!");

    // No need to check if the client is disconnected, because we
    // only accept one peer at a time.

    m_peer = new player::Peer{ peer };

    if (m_client && m_client->is_redirecting()) {
        m_client->redirect();
    }
    else {
        // Request server data using saved headers and params from the growtopia client.
        utils::TextParse text_parse{ m_http->request_server_data() }; // TODO: Handle crash.
        std::string host = text_parse.get("server", 1);
        auto port = text_parse.get<enet_uint16>("port", 1);

        // Check if server using new packet.
        m_config->m_server.using_new_packet = text_parse.get<int>("type2", 1) == 1;
        if (text_parse.get<int>("type2", 1) == 1) {
            spdlog::debug("Server is using new packet.");
        }

        m_client = new client::Client{ m_config, this };
        if (!m_client->start(host, port)) {
            spdlog::error("Failed to connect to client!");

            delete m_client;
            m_client = nullptr;
        }
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

    if (!process_packet(peer, packet))
        return;

    m_client->get_peer()->send_packet_packet(packet);
}

void Server::on_disconnect(ENetPeer* peer)
{
    spdlog::info("Client disconnected from proxy server!");

    if (m_client && !m_client->is_redirecting()) {
        if (m_client->get_peer() && m_client->get_peer()->is_connected()) {
            // Yes, useless log.
            spdlog::info("Disconnected from growtopia server!");
        }

        // The client will disconnect now from growtopia server after
        // deleting the client object.
        delete m_client;
        m_client = nullptr;
    }

    delete m_peer;
    m_peer = nullptr;

    if (m_client && m_client->is_redirecting()) {
        spdlog::info("Redirecting to another server!");
    }
}

bool Server::process_packet(ENetPeer* peer, ENetPacket* packet)
{
    player::eNetMessageType message_type{ player::message_type_to_string(packet) };
    std::string message_data{ player::get_text(packet) };

    if (message_type != player::NET_MESSAGE_GAME_PACKET) {
        utils::TextParse text_parse{ message_data };
        if (!text_parse.empty()) {
            spdlog::info("Outgoing MessagePacket:\n{} [{}]:\n{}\n", magic_enum::enum_name(message_type),
                message_type, fmt::join(text_parse.get_all_array(), "\r\n"));
        }
    }

    switch (message_type) {
    case player::NET_MESSAGE_GENERIC_TEXT: {
        if (message_data.find("requestedName") == std::string::npos) {
            break;
        }

        static randutils::pcg_rng gen{ utils::random::get_generator_local() };
        static std::string mac{ utils::random::generate_mac(gen) };
        static uint32_t mac_hash{ utils::proton_hash(fmt::format("{}RT", mac).c_str()) };
        static std::string rid{ utils::random::generate_hex(gen, 16, true) };
        static std::string wk{ utils::random::generate_hex(gen, 16, true) };
        static std::string device_id{ utils::random::generate_hex(gen, 16, true) };
        static uint32_t device_id_hash{ utils::proton_hash(fmt::format("{}RT", device_id).c_str()) };

        utils::TextParse text_parse{ message_data };

        if (m_config->m_misc.force_update_game_version) {
            if (std::stoi(text_parse.get("game_version", 1)) > std::stoi(m_config->m_server.game_version)) {
                m_config->m_server.game_version = text_parse.get("game_version", 1);
            }
        }

        text_parse.set("game_version", m_config->m_server.game_version);
        text_parse.set("protocol", m_config->m_server.protocol);
        text_parse.set("mac", mac);
        text_parse.set("rid", rid);
        text_parse.set("wk", wk);
        text_parse.set("hash", device_id_hash);
        text_parse.set("hash2", mac_hash);

        m_client->get_peer()->send_packet(message_type, text_parse.get_all_raw());
        return false;
    }
    case player::NET_MESSAGE_GAME_PACKET: {
        player::GameUpdatePacket* game_update_packet{ player::get_struct(packet) };
        return process_tank_update_packet(peer, game_update_packet);
    }
    default:break;
    }

    return true;
}

bool Server::process_tank_update_packet(ENetPeer* peer, player::GameUpdatePacket* game_update_packet)
{
    if (game_update_packet->type != player::PACKET_STATE) {
        uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
        std::vector<uint8_t> extended_data_vector{ extended_data, extended_data + game_update_packet->data_size };
        spdlog::info("Outgoing TankUpdatePacket:\n [{}]{}{}", game_update_packet->type,
            magic_enum::enum_name(static_cast<player::ePacketType>(game_update_packet->type)),
            extended_data ? fmt::format("\n > extended_data: {}", spdlog::to_hex(extended_data_vector)) : "");
    }

    switch (game_update_packet->type) {
    case player::PACKET_DISCONNECT:m_peer->disconnect_now();
        on_disconnect(peer);
        break;
    default:break;
    }

    return true;
}
}
