#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <util/Variant.h>

#include "client.h"
#include "../server/server.h"
#include "../utils/textparse.h"

namespace client {
    Client::Client(server::Server *server)
        : enetwrapper::ENetClient()
        , m_proxy_server(server)
        , m_player(nullptr)
    {
        m_send_server_info = new SendServerInfo{};
    }

    Client::~Client() {
        delete m_player;
        delete m_send_server_info;
    }

    bool Client::initialize() {
        if (!connect("213.179.209.168", 17254, 1)) {
            return false;
        }

        start_service();
        return true;
    }

    void Client::on_connect(ENetPeer *peer) {
        spdlog::info("Client connected to growtopia server: {}", peer->connectID);
        m_player = new player::Player{ peer };
    }

    void Client::on_receive(ENetPeer *peer, ENetPacket *packet) {
        spdlog::info("Received packet from growtopia server: {} bytes", packet->dataLength);

        if (m_proxy_server == nullptr) {
            return;
        }

        if (m_player == nullptr) {
            return;
        }

        m_send_server_info->check = false;

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

                    if (variant_list.Get(0).GetString() == "OnSendToServer") {
                        std::vector<std::string> data{ utils::TextParse::string_tokenize(variant_list.Get(4).GetString()) };

                        m_send_server_info->port = variant_list.Get(1).GetINT32();
                        m_send_server_info->token = variant_list.Get(2).GetINT32();
                        m_send_server_info->user = variant_list.Get(3).GetINT32();
                        m_send_server_info->host = data.at(0);
                        m_send_server_info->uuid_token = data.size() == 2 ? data.at(1) : data.at(2);
                        m_send_server_info->check = true;

                        m_proxy_server->get_player()->send_variant({
                            "OnSendToServer",
                            17000,
                            m_send_server_info->token,
                            m_send_server_info->user,
                            fmt::format("127.0.0.1|{}|{}", data.size() == 2 ? "" : data.at(1), m_send_server_info->uuid_token),
                            variant_list.Get(5).GetINT32()
                        });
                        return;
                    }
                }
            }
            else {
                uint8_t *extended_data{ player::get_extended_data(game_update_packet) };

                if (extended_data) {
                    std::vector<char> extended_data_int;
                    for (int i = 0; i < game_update_packet->data_extended_size; i++) {
                        extended_data_int.push_back(static_cast<char>(extended_data[i]));
                    }

                    spdlog::debug("Received packet from growtopia server hex: {}", spdlog::to_hex(extended_data_int));
                }
            }
        }
        else {
            spdlog::debug("{}: {}", message_type, message_data);
        }

        if (m_proxy_server->get_player()->send_packet_packet(packet) != 0) {
            spdlog::error("Failed to send packet to growtopia client");
        }
        else {
            spdlog::debug("Sent packet to growtopia client");
        }
    }

    void Client::on_disconnect(ENetPeer *peer) {
        spdlog::info("Client disconnected from growtopia server: (peer->data! {})", peer->data);

        if (!peer->data) {
            return;
        }

        if (m_player->get_peer()) {
            enet_peer_disconnect(m_player->get_peer(), 0);
        }

        delete m_player;
        m_player = nullptr;

        if (m_proxy_server->get_player()) {
            enet_peer_disconnect(m_proxy_server->get_player()->get_peer(), 0);
        }

        m_proxy_server = nullptr;
    }
}
