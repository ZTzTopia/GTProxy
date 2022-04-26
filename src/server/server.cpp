#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <util/Variant.h>

#include "server.h"
#include "../utils/random.h"
#include "../utils/textparse.h"

namespace server {
    Server::Server()
        : enetwrapper::ENetServer()
        , m_proxy_client(nullptr)
        , m_player(nullptr)
        , m_login_info()
    {
        m_command_handler = new command::CommandHandler{ this };
    }

    Server::~Server() {
        delete m_proxy_client;
        delete m_player;
    }

    bool Server::initialize() {
        if (!create_host(17000, 1)) {
            return false;
        }

        start_service();
        return true;
    }

    void Server::on_connect(ENetPeer *peer) {
        spdlog::info("Client connected to proxy server: {}", peer->connectID);

        if (!m_proxy_client || !m_proxy_client->get_send_server_info()->check) {
            delete m_proxy_client;

            // Start proxy client.
            m_proxy_client = new client::Client{ this };
            if (!m_proxy_client->initialize()) {
                spdlog::error("Failed to initialize proxy client.");

                delete m_proxy_client;
                m_proxy_client = nullptr;
            }
        }
        else {
            std::string prot{ m_proxy_client->get_send_server_info()->host };
            uint32_t prottt{ m_proxy_client->get_send_server_info()->port };

            delete m_proxy_client;
            m_proxy_client = new client::Client{ this };
            m_proxy_client->connect(prot, prottt, 1);
            m_proxy_client->start_service();

            m_proxy_client->get_send_server_info()->check = false;
        }

        m_player = new player::Player{ peer };
    }

    uint32_t hash_string(const char *data, uint32_t length) {
        uint32_t hash = 0x55555555;
        if (data) {
            if (length >= 1) {
                while (length) {
                    hash = (hash >> 27) + (hash << 5) + *reinterpret_cast<const uint8_t *>(data++);
                    length--;
                }
            }
            else {
                while (*data) {
                    hash = (hash >> 27) + (hash << 5) + *reinterpret_cast<const uint8_t*>(data++);
                }
            }
        }

        return hash;
    }

    void Server::on_receive(ENetPeer *peer, ENetPacket *packet) {
        if (!m_proxy_client || !m_player)
            return;

        player::eNetMessageType message_type{ player::get_message_type(packet) };
        std::string message_data{ player::get_text(packet) };
        switch(message_type) {
            case player::NET_MESSAGE_GENERIC_TEXT:
            case player::NET_MESSAGE_GAME_MESSAGE: {
                if (message_data.find("requestedName") != std::string::npos) {
                    utils::TextParse text_parse{ message_data };
                    if (!text_parse.get("requestedName", 1).empty()) {
                        randutils::pcg_rng gen{ utils::random::get_generator_static() };

                        static std::string mac{ utils::random::generate_mac(gen) };
                        static std::string rid{ utils::random::generate_hex(gen, 16, true) };
                        static std::string wk{ utils::random::generate_hex(gen, 16, true) };
                        static std::string device_id{ utils::random::generate_hex(gen, 16, true) };

                        text_parse.set("protocol", 160);
                        text_parse.set("game_version", "3.86");
                        text_parse.set("mac", mac);
                        text_parse.set("rid", rid);
                        text_parse.set("wk", wk);
                        text_parse.set("hash", hash_string(device_id.c_str(), 0));
                        text_parse.set("hash2", hash_string(mac.c_str(), 0));

                        m_proxy_client->get_player()->send_packet(message_type, text_parse.get_all_raw());
                        return;
                    }
                }
                else if (message_data.find("action|input") != std::string::npos) {
                    utils::TextParse text_parse{ message_data };
                    if (!text_parse.get("text", 1).empty()) {
                        if (m_command_handler->handle(text_parse.get("text", 1)))
                            return;
                    }
                }
                break;
            }
            case player::NET_MESSAGE_GAME_PACKET: {
                player::GameUpdatePacket *updatePacket{ player::get_struct(packet) };
                if(!updatePacket)
                    return;
                switch(updatePacket->type) {
                    case player::PACKET_CALL_FUNCTION: {
                        uint8_t *extended_data{ player::get_extended_data(updatePacket) };
                        if (!extended_data)
                            break;
                        VariantList variant_list{};
                        variant_list.SerializeFromMem(extended_data, static_cast<int>(updatePacket->data_size));
                        spdlog::info("{}", variant_list.GetContentsAsDebugString());
                        break;
                    }
                    case player::PACKET_DISCONNECT: {
                        enet_peer_disconnect_now(peer, 0);
                        break;
                    }
                    default: {
                        if(updatePacket->type == player::PACKET_STATE) {
                            m_proxy_client->get_player()->get_avatar()->pos = CL_Vec2f{ updatePacket->pos_x, updatePacket->pos_y };
                            break;
                        }
                        uint8_t *extended_data{ player::get_extended_data(updatePacket) };
                        std::vector<char> data_array;
                        for (uint32_t i = 0; i < updatePacket->data_size; i++)
                            data_array.push_back(static_cast<char>(extended_data[i]));
                        spdlog::info("Outgoing GameUpdatePacket:\n [{}]{}{}", 
                            updatePacket->type, 
                            player::get_packet_type(updatePacket->type),
                            extended_data ? fmt::format("\n > extended_data: {}", spdlog::to_hex(data_array)) : "");
                        break;
                    }
                }
                break;
            }
            default: {
                spdlog::info("[{}]{}: {}", message_type, player::get_message_type(message_type), message_data);
                break;
            }
        }
        if (m_proxy_client->get_player()->send_packet_packet(packet) != 0)
            spdlog::error("Failed to send packet to growtopia server");
    }

    void Server::on_disconnect(ENetPeer *peer) {
        spdlog::info("Client disconnected from Growtopia Client: (peer->data! -> {})", peer->data);

        if (!peer->data)
            return;
        if (m_player->get_peer())
            enet_peer_disconnect(m_player->get_peer(), 0);       
        delete m_player;
        m_player = nullptr;

        if (m_proxy_client && m_proxy_client->get_player())
            enet_peer_disconnect(m_proxy_client->get_player()->get_peer(), 0);
    }
}
