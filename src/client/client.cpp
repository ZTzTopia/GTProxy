#include <spdlog/fmt/bin_to_hex.h>

#include "client.h"
#include "../server/server.h"
#include "../utils/hash.h"
#include "../utils/text_parse.h"

namespace client {
    Client::Client(Config* config, server::Server* server) : enetwrapper::ENetClient{}, m_config{ config }, m_peer{ nullptr }, m_server{ server }, m_redirecting{ false } {}

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
        m_redirecting = false;
    }

    void Client::on_receive(ENetPeer* peer, ENetPacket* packet)
    {
        if (!m_server->get_peer())
            return;

        if (!m_server->get_peer()->is_connected())
            return;

        if (!process_packet(peer, packet))
            return;

        m_server->get_peer()->send_packet_packet(packet);
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
        player::eNetMessageType message_type{ player::message_type_to_string(packet) };
        std::string message_data{ player::get_text(packet) };

        if (message_type != player::NET_MESSAGE_GAME_PACKET) {
            utils::TextParse text_parse{ message_data };
            if (!text_parse.empty()) {
                spdlog::info("Incoming MessagePacket:\n{} [{}]:\n{}\n", player::message_type_to_string(message_type), message_type, fmt::join(text_parse.get_all_array(), "\r\n"));
            }
        }

        switch (message_type) {
            case player::NET_MESSAGE_GAME_PACKET: {
                player::GameUpdatePacket* game_update_packet{ player::get_struct(packet) };
                return process_tank_update_packet(peer, game_update_packet);
            }
            default:
                break;
        }

        return true;
    }

    bool Client::process_tank_update_packet(ENetPeer* peer, player::GameUpdatePacket* game_update_packet)
    {
        if (game_update_packet->type != player::PACKET_STATE && game_update_packet->type != player::PACKET_CALL_FUNCTION) {
            uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
            std::vector<uint8_t> extended_data_vector{ extended_data, extended_data + game_update_packet->data_size };
            spdlog::info("Incoming TankUpdatePacket:\n [{}]{}{}", game_update_packet->type, player::packet_type_to_string(game_update_packet->type), extended_data ? fmt::format("\n > extended_data: {}", spdlog::to_hex(extended_data_vector)) : "");
        }

        switch (game_update_packet->type) {
            case player::PACKET_CALL_FUNCTION: {
                uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
                if (!extended_data) break;

                VariantList variant_list{};
                variant_list.SerializeFromMem(extended_data, static_cast<int>(game_update_packet->data_size));

                spdlog::info("Incoming VariantList:\n{}", variant_list.GetContentsAsDebugString());

                std::size_t hash{ utils::fnv1a_hash(variant_list.Get(0).GetString()) };
                switch (hash) {
                    case "OnSendToServer"_fh: {
                        std::vector<std::string> tokenize{ utils::TextParse::string_tokenize(variant_list.Get(4).GetString()) };
                        m_redirecting = true;
                        m_redirect_host = std::move(tokenize[0]);
                        m_redirect_port = static_cast<enet_uint16>(variant_list.Get(1).GetINT32());

                        m_server->get_peer()->send_variant({ "OnSendToServer", 17000, variant_list.Get(2).GetINT32(), variant_list.Get(3).GetINT32(), fmt::format("127.0.0.1|{}|{}", tokenize.size() == 2 ? "" : tokenize.at(1), tokenize.size() == 2 ? tokenize.at(1) : tokenize.at(2)), variant_list.Get(5).GetINT32() });
                        return false;
                    }
                    default:
                        break;
                }

                break;
            }
            default:
                break;
        }

        return true;
    }
}
