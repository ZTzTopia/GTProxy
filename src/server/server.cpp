#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <util/Variant.h>

#include "server.h"
#include "../config.h"
#include "../utils/random.h"
#include "../utils/textparse.h"
#include "../utils/quick_hash.h"

namespace server {
    Server::Server()
        : enetwrapper::ENetServer(), m_proxy_client(nullptr), m_player(nullptr)
    {
        m_command_handler = new command::CommandHandler{ this };
    }

    Server::~Server()
    {
        delete m_proxy_client;
        delete m_player;
    }

    bool Server::initialize()
    {
        if (!create_host(17000, 1))
            return false;

        start_service();
        return true;
    }

    void Server::on_connect(ENetPeer *peer) {
        spdlog::info("Client connected to proxy server: {}", peer->connectID);

        // TODO: Fix all about client and server disconnecting.
        if (!m_proxy_client) {
messy_code:
            m_proxy_client = new client::Client{ this };
            if (!m_proxy_client->initialize()) {
                spdlog::error("Failed to initialize proxy client.");

                delete m_proxy_client;
                m_proxy_client = nullptr;
            }
        }
        else {
            if (m_proxy_client->is_on_send_to_server()) {
                m_proxy_client->connect(m_proxy_client->get_host(), m_proxy_client->get_port(), 1);
                m_proxy_client->start_service();
            }
            else {
                delete m_proxy_client;
                goto messy_code;
            }
        }

        m_player = new player::Player{ peer };
    }

    void Server::on_receive(ENetPeer *peer, ENetPacket *packet) {
        if (!m_player || !m_proxy_client)
            return;

        player::eNetMessageType message_type{ player::message_type_to_string(packet) };
        std::string message_data{ player::get_text(packet) };
        switch (message_type) {
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

                        text_parse.set("protocol", Config::get().config()["server"]["protocol"].get<uint8_t>());
                        text_parse.set("game_version", Config::get().config()["server"]["gameVersion"]);
                        text_parse.set("mac", mac);
                        text_parse.set("rid", rid);
                        text_parse.set("wk", wk);
                        text_parse.set("hash", utils::proton_hash(device_id.c_str(), 0));
                        text_parse.set("hash2", utils::proton_hash(mac.c_str(), 0));

                        if (m_proxy_client->get_player())
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
                else if (message_data.find("action|quit") != std::string::npos &&
                    message_data.find("action|quit_to_exit") == std::string::npos) {
                    enet_peer_disconnect_now(peer, 0);
                }
                break;
            }
            case player::NET_MESSAGE_GAME_PACKET: {
                player::GameUpdatePacket* updatePacket{ player::get_struct(packet) };
                if (!updatePacket)
                    return;

                switch(updatePacket->type) {
                    case player::PACKET_STATE: {
                        spdlog::debug("flags: {}", player::flag_to_string(
                            static_cast<player::ePacketFlag>(updatePacket->flags)));

                        if (m_proxy_client->get_player())
                            m_proxy_client->get_player()->get_avatar()->pos = { updatePacket->pos_x, updatePacket->pos_y };
                    }
                    case player::PACKET_CALL_FUNCTION: {
                        uint8_t* extended_data{ player::get_extended_data(updatePacket) };
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
                        uint8_t* extended_data{ player::get_extended_data(updatePacket) };

                        std::vector<char> data_array;
                        for (uint32_t i = 0; i < updatePacket->data_size; i++)
                            data_array.push_back(static_cast<char>(extended_data[i]));

                        spdlog::info("Outgoing GameUpdatePacket:\n [{}]{}{}", 
                            updatePacket->type, 
                            player::packet_type_to_string(updatePacket->type),
                            extended_data ? fmt::format("\n > extended_data: {}", spdlog::to_hex(data_array)) : "");
                        break;
                    }
                }
                break;
            }
            default: {
                spdlog::info("[{}]{}: {}", message_type, player::message_type_to_string(message_type), message_data);
                break;
            }
        }

        if (m_proxy_client->get_player() && m_proxy_client->get_player()->send_packet_packet(packet) != 0)
            spdlog::error("Failed to send packet to growtopia server");

        enet_host_flush(m_host);
    }

    void Server::on_disconnect(ENetPeer *peer) {
        spdlog::info("Client disconnected from Growtopia Client: (peer->data! -> {})", peer->data);

        if (!m_player) return;
        if (m_player->get_peer())
            enet_peer_disconnect_now(m_player->get_peer(), 0);

        delete m_player;
        m_player = nullptr;

        if (m_proxy_client && m_proxy_client->get_player())
            enet_peer_disconnect_now(m_proxy_client->get_player()->get_peer(), 0);
    }
}
