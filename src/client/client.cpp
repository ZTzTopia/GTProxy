#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <util/Variant.h>
#include <httplib.h>
#include "fmt/ranges.h"

#include "client.h"
#include "../server/server.h"
#include "../utils/textparse.h"
#include "../utils/quick_hash.h"

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
        // Get server and port from growtopia1.com
        httplib::Client http_client{ "http://13.248.211.25" };
        httplib::Result response = http_client.Post("/growtopia/server_data.php");
        if (response.error() != httplib::Error::Success || response->status != 200) {
            spdlog::error("Failed to get server data. HTTP status code: {}", response->status);
            return false;
        }
        utils::TextParse text_parse{ response->body };
        std::string server{ text_parse.get("server", 1) };
        enet_uint16 port{ text_parse.get<enet_uint16>("port", 1) };

        if (!connect(server, port, 1))
            return false;
        start_service();
        return true;
    }

    void Client::on_connect(ENetPeer *peer) {
        spdlog::info("Client connected to growtopia server: {}", peer->connectID);
        m_player = new player::Player{ peer };
    }

    void Client::on_receive(ENetPeer *peer, ENetPacket *packet) {
        if (!m_proxy_server || !m_player)
            return;
        m_send_server_info->check = false;

        player::eNetMessageType message_type{ player::get_message_type(packet) };
        std::string message_data{ player::get_text(packet) };
        switch(message_type) {
            case player::NET_MESSAGE_GAME_PACKET: {
                player::GameUpdatePacket *updatePacket{player::get_struct(packet)};
                uint8_t* extended_data{ player::get_extended_data(updatePacket) };
                if(!extended_data)
                    break;
                switch (updatePacket->packet_type) {
                    case player::PACKET_CALL_FUNCTION: {
                        VariantList variant_list{};
                        variant_list.SerializeFromMem(extended_data, static_cast<int>(updatePacket->data_extended_size));
                        
                        using namespace utils;
                        const auto& hash = utils::quick_hash(variant_list.Get(0).GetString());
                        switch (hash) {
                            case "OnSendToServer"_qh: {
                                std::vector<std::string> tokenize{ utils::TextParse::string_tokenize(variant_list.Get(4).GetString()) };

                                m_send_server_info->port = variant_list.Get(1).GetINT32();
                                m_send_server_info->token = variant_list.Get(2).GetINT32();
                                m_send_server_info->user = variant_list.Get(3).GetINT32();
                                m_send_server_info->host = tokenize.at(0);
                                m_send_server_info->uuid_token = tokenize.size() == 2 ? tokenize.at(1) : tokenize.at(2);
                                m_send_server_info->check = true;

                                m_proxy_server->get_player()->send_variant({
                                    "OnSendToServer",
                                    17000,
                                    m_send_server_info->token,
                                    m_send_server_info->user,
                                    fmt::format("127.0.0.1|{}|{}", tokenize.size() == 2 ? "" : tokenize.at(1), m_send_server_info->uuid_token),
                                    variant_list.Get(5).GetINT32()
                                });
                                return;
                            }
                            case "onShowCaptcha"_qh: {
                                std::vector<std::string> tokenize{ utils::TextParse::string_tokenize(variant_list.Get(1).GetString()) };
                                for (uint32_t i = 0; i < tokenize.size(); i++) {
                                    if (tokenize[i].find(" + ") != std::string::npos) {
                                        std::vector<std::string> tokenize_{ utils::TextParse::string_tokenize(tokenize[i], " + ") };
                                        uint8_t sum_1 = std::atoi(tokenize_[0].c_str());
                                        uint8_t sum_2 = std::atoi(tokenize_[1].c_str());
                                        uint8_t sum = sum_1 + sum_2;
                                        m_proxy_server->get_player()->send_log(fmt::format("Solved captcha: {} + {} = {}", sum_1, sum_2, sum));
                                        m_player->send_packet(player::NET_MESSAGE_GENERIC_TEXT, 
                                            fmt::format("action|dialog_return\n"
                                                "dialog_name|captcha_submit\n"
                                                "captcha_answer|{}", 
                                            sum));
                                        return;
                                    }
                                }
                                return;
                            }
                            default: {
                                spdlog::info("Incoming VariantList:\n{}", variant_list.GetContentsAsDebugString());
                                break;
                            }
                        }
                        break;
                    }
                    case player::PACKET_APP_INTEGRITY_FAIL:
                        return;
                    default: {
                        spdlog::info("Incoming GameUpdatePacket:\n [{}]{}", updatePacket->packet_type, player::get_packet_type(updatePacket->packet_type));
                        uint8_t *extended_data{ player::get_extended_data(updatePacket) };
                        if (!extended_data)
                            break;
                        std::vector<char> data_array;
                        for (uint32_t i = 0; i < updatePacket->data_extended_size; i++)
                            data_array.push_back(static_cast<char>(extended_data[i]));
                        spdlog::info(" > extended_data: {}", spdlog::to_hex(data_array));
                        break;
                    }
                }
                break;
            }
            default: {
                utils::TextParse text_parse{ message_data };
                if(text_parse.empty())
                    break;
                spdlog::info("Incoming TankUpdatePacket:\n{} [{}]:\n{}\n", 
                    player::get_message_type(message_type),
                    message_type,
                    fmt::join(text_parse.get_all_array(), "\r\n"));
                break;
            }
        }
        if (m_proxy_server->get_player()->send_packet_packet(packet) != 0)
            spdlog::error("Failed to send packet to growtopia client");
    }

    void Client::on_disconnect(ENetPeer *peer) {
        spdlog::info("Client disconnected from Growtopia Server: (peer->data! -> {})", peer->data);

        if (!peer->data)
            return;
        if (m_player->get_peer())
            enet_peer_disconnect(m_player->get_peer(), 0);
        delete m_player;
        m_player = nullptr;

        if (m_proxy_server->get_player())
            enet_peer_disconnect(m_proxy_server->get_player()->get_peer(), 0);
        m_proxy_server = nullptr;
    }
}
