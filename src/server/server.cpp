#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <util/Variant.h>

#include "server.h"
#include "../config.h"
#include "../utils/random.h"
#include "../utils/textparse.h"
#include "../utils/hash.h"

namespace server {
    Server::Server()
        : enetwrapper::ENetServer(), m_client(nullptr), m_player(nullptr)
    {
        m_command_handler = new command::CommandHandler{ this };
    }

    Server::~Server()
    {
        delete m_client;
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
        if (!m_client) {
messy_code:
            m_client = new client::Client{ this };
            if (!m_client->initialize()) {
                spdlog::error("Failed to initialize proxy client.");

                delete m_client;
                m_client = nullptr;
            }
        }
        else {
            if (m_client->is_on_send_to_server()) {
                m_client->create_host(1);
                m_client->connect(m_client->get_host(), m_client->get_port());
                m_client->start_service();
            }
            else {
                delete m_client;
                goto messy_code;
            }
        }

        m_player = new player::Player{ peer };
    }

    void Server::on_receive(ENetPeer *peer, ENetPacket *packet) {
        if (!peer || !packet || packet->dataLength < 5) return;
        if (!m_player || !m_client) return;

        bool process{ process_packet(peer, packet) };

        if (process && m_client->get_player()->send_packet_packet(packet) != 0)
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

        if (m_client && m_client->get_player())
            enet_peer_disconnect_now(m_client->get_player()->get_peer(), 0);
    }

    bool Server::process_packet(ENetPeer* peer, ENetPacket* packet)
    {
        player::eNetMessageType message_type{ player::message_type_to_string(packet) };
        std::string message_data{ player::get_text(packet) };
        switch (message_type) {
            case player::NET_MESSAGE_GENERIC_TEXT:
            case player::NET_MESSAGE_GAME_MESSAGE: {
                if (message_data.find("requestedName") != std::string::npos) {
                    utils::TextParse text_parse{ message_data };
                    if (!text_parse.get("requestedName", 1).empty()) {
                        static randutils::pcg_rng gen{ utils::random::get_generator_static() };
                        static std::string mac{ utils::random::generate_mac(gen) };
                        static std::string rid{ utils::random::generate_hex(gen, 16, true) };
                        static std::string wk{ utils::random::generate_hex(gen, 16, true) };
                        static std::string device_id{ utils::random::generate_hex(gen, 16, true) };

                        auto protocol{ Config::get().config()["server"]["protocol"].get<uint8_t>() };
                        if (text_parse.get<uint8_t>("protocol", 1) < protocol)
                            text_parse.set("protocol", protocol);

                        std::string version{ Config::get().config()["server"]["gameVersion"] };
                        version.erase(std::remove(version.begin(), version.end(), '.'), version.end());
                        if (text_parse.get<uint32_t>("game_version", 1) < std::stoi(version))
                            text_parse.set("game_version", Config::get().config()["server"]["gameVersion"]);

                        text_parse.set("mac", mac);
                        text_parse.set("rid", rid);
                        text_parse.set("wk", wk);
                        text_parse.set("hash", utils::proton_hash(fmt::format("{}RT", device_id).c_str(), 0));
                        text_parse.set("hash2", utils::proton_hash(fmt::format("{}RT", mac).c_str(), 0));

                        if (m_client->get_player())
                            m_client->get_player()->send_packet(message_type, text_parse.get_all_raw());

                        enet_host_flush(m_host);
                        return false;
                    }
                }
                else if (message_data.find("action|input") != std::string::npos) {
                    utils::TextParse text_parse{ message_data };
                    if (!text_parse.get("text", 1).empty()) {
                        if (m_command_handler->handle(text_parse.get("text", 1)))
                            return false;
                    }
                }
                else if (message_data.find("action|wrench") != std::string::npos) {
                    utils::TextParse text_parse{ message_data };
                    auto net_id = text_parse.get<uint32_t>("netid", 1);

                    player::LocalPlayer* local_player{ m_client->get_local_player() };
                    if (net_id != local_player->get_net_id()) {
                        std::string button_clicked{};
                        if (local_player->has_flags(player::eFlag::FAST_WRENCH_PULL))
                            button_clicked = "pull";
                        else if (local_player->has_flags(player::eFlag::FAST_WRENCH_KICK))
                            button_clicked = "kick";
                        else if (local_player->has_flags(player::eFlag::FAST_WRENCH_BAN))
                            button_clicked = "worldban";

                        if (!button_clicked.empty()) {
                            m_client->get_player()->send_packet(
                                player::NET_MESSAGE_GENERIC_TEXT,
                                fmt::format(
                                    "action|dialog_return\n"
                                    "dialog_name|popup\n"
                                    "netID|{}\n"
                                    "buttonClicked|{}", net_id, button_clicked));

                            enet_host_flush(m_host);
                            return false;
                        }
                    }
                }
                else if (message_data.find("action|quit") != std::string::npos &&
                    message_data.find("action|quit_to_exit") == std::string::npos) {
                    enet_peer_disconnect_now(peer, 0);
                }
                else {
                    spdlog::info("[{}]{}:\n{}", player::message_type_to_string(message_type), message_type, message_data);
                }
                break;
            }
            case player::NET_MESSAGE_GAME_PACKET: {
                player::GameUpdatePacket* game_update_packet{ player::get_struct(packet) };
                return process_tank_update_packet(peer, game_update_packet);
            }
            default:
                spdlog::info("[{}]{}: {}", message_type, player::message_type_to_string(message_type), message_data);
                break;
        }

        return true;
    }

    bool Server::process_tank_update_packet(ENetPeer* peer, player::GameUpdatePacket* game_update_packet)
    {
        switch(game_update_packet->type) {
            case player::PACKET_STATE: {
                player::LocalPlayer* local_player{ m_client->get_local_player() };
                local_player->set_pos({
                    static_cast<int>(game_update_packet->pos_x),
                    static_cast<int>(game_update_packet->pos_y) });

                if (game_update_packet->item_id != 0) {
                    PlayerItems* inventory = get_client()->get_local_player()->get_items();
                    for (auto& item: inventory->items) {
                        if (item.first == game_update_packet->item_id) {
                            item.second.first -= 1;
                            if (item.second.first <= 0)
                                inventory->items.erase(item.first);
                            break;
                        }
                    }
                }
                break;
            }
            case player::PACKET_CALL_FUNCTION: {
                uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
                if (!extended_data) break;

                VariantList variant_list{};
                variant_list.SerializeFromMem(extended_data, static_cast<int>(game_update_packet->data_size));
                spdlog::info("{}", variant_list.GetContentsAsDebugString());
                break;
            }
            case player::PACKET_DISCONNECT:
                enet_peer_disconnect_now(peer, 0);
                break;
            default: {
                uint8_t* extended_data{ player::get_extended_data(game_update_packet) };

                std::vector<char> data_array;
                for (uint32_t i = 0; i < game_update_packet->data_size; i++)
                    data_array.push_back(static_cast<char>(extended_data[i]));

                spdlog::info(
                    "Outgoing TankUpdatePacket:\n [{}]{}{}",
                    game_update_packet->type,
                    player::packet_type_to_string(game_update_packet->type),
                    extended_data ? fmt::format("\n > extended_data: {}", spdlog::to_hex(data_array)) : "");
                break;
            }
        }

        return true;
    }
}
