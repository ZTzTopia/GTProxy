#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>

#include "server.h"
#include "../utils/random.h"
#include "../utils/textparse.h"
#include "util/Variant.h"

namespace server {
    Server::Server()
        : enetwrapper::ENetServer()
        , m_proxy_client(nullptr)
        , m_player(nullptr)
        , m_login_info()
    {}

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
        spdlog::info("Received packet from growtopia client: {} bytes", packet->dataLength);

        if (m_proxy_client == nullptr) {
            return;
        }

        if (m_player == nullptr) {
            return;
        }

        player::eNetMessageType message_type{ player::get_message_type(packet) };
        std::string message_data{ player::get_text(packet) };

        if (message_type == player::NET_MESSAGE_GAME_PACKET) {
            player::GameUpdatePacket *game_update_packet{ player::get_struct(packet) };
            spdlog::debug("gameudpatepacket type: {}", game_update_packet->packet_type);
            spdlog::debug("{}, {}, {}", game_update_packet->unk1, game_update_packet->unk2, game_update_packet->unk3);
            spdlog::debug("{}, {}, {}", game_update_packet->net_id, game_update_packet->unk5, game_update_packet->flags);
            spdlog::debug("{}, {}, {}", game_update_packet->object_amount, game_update_packet->dec_item_data_size, game_update_packet->pos_x);
            spdlog::debug("{}, {}, {}", game_update_packet->pos_y, game_update_packet->unk11, game_update_packet->unk12);
            spdlog::debug("{}, {}, {}", game_update_packet->unk13, game_update_packet->m_tile_pos_x, game_update_packet->m_tile_pos_y);

            if (game_update_packet->packet_type == player::PACKET_CALL_FUNCTION) {
                uint8_t *extended_data{ player::get_extended_data(game_update_packet) };
                if (extended_data) {
                    VariantList variant_list{};
                    variant_list.SerializeFromMem(extended_data, static_cast<int>(game_update_packet->data_extended_size));

                    spdlog::debug("{}", variant_list.GetContentsAsDebugString());
                }
            }
            else if (game_update_packet->packet_type == player::PACKET_DISCONNECT) {
                enet_peer_disconnect_now(peer, 0);
            }
            else {
                uint8_t *extended_data{ player::get_extended_data(game_update_packet) };

                if (extended_data) {
                    std::vector<char> extended_data_int;
                    for (int i = 0; i < game_update_packet->data_extended_size; i++) {
                        extended_data_int.push_back(static_cast<char>(extended_data[i]));
                    }

                    spdlog::debug("Extended data from growtopia client hex: {}", spdlog::to_hex(extended_data_int));
                }
            }
        }
        else {
            spdlog::debug("{}: {}", message_type, message_data);
        }

        if (message_data.find("action|quit") != std::string::npos) {
            enet_peer_disconnect_later(peer, 0);
            return;
        }

        utils::TextParse text_parse{ message_data };
        if (!text_parse.get("requestedName", 1).empty()) {
            randutils::pcg_rng gen{ utils::random::get_generator_static() };

            static std::string mac{ utils::random::generate_mac(gen) };
            static std::string rid{ utils::random::generate_hex(gen, 16, true) };
            static std::string wk{ utils::random::generate_hex(gen, 16, true) };
            static std::string device_id{ utils::random::generate_hex(gen, 16, true) };

            text_parse.set("mac", mac);
            text_parse.set("rid", rid);
            text_parse.set("wk", wk);
            text_parse.set("hash", hash_string(device_id.c_str(), 0));
            text_parse.set("hash2", hash_string(mac.c_str(), 0));

            m_proxy_client->get_player()->send_packet(message_type, text_parse.get_all_raw());
            return;
        }

        if (m_proxy_client->get_player()->send_packet_packet(packet) != 0) {
            spdlog::error("Failed to send packet to growtopia server");
        }
        else {
            spdlog::debug("Sent packet to growtopia server");
        }
    }

    void Server::on_disconnect(ENetPeer *peer) {
        spdlog::info("Client disconnected from growtopia client: (peer->data! {})", peer->data);

        if (!peer->data) {
            return;
        }

        if (m_player->get_peer()) {
            enet_peer_disconnect(m_player->get_peer(), 0);
        }

        delete m_player;
        m_player = nullptr;

        if (m_proxy_client && m_proxy_client->get_player()) {
            enet_peer_disconnect(m_proxy_client->get_player()->get_peer(), 0);
        }
    }
}
