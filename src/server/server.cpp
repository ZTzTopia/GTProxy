#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <util/Variant.h>

#include "server.h"
#include "../config.h"
#include "../utils/hash.h"
#include "../utils/random.h"
#include "../utils/text_parse.h"

namespace server {
    Server::Server()
        : enetwrapper::ENetServer(), m_client(nullptr), m_player(nullptr), m_disconnected(true)
    {
        m_command_manager = new command::CommandManager{};
    }

    Server::~Server()
    {
        delete m_client;
        delete m_player;
        delete m_command_manager;
    }

    bool Server::initialize()
    {
        if (!create_host(17000, 1))
            return false;

        start_service();
        return true;
    }

    void Server::on_connect(ENetPeer *peer)
    {
        spdlog::info("Client connected to proxy server: {}", peer->connectID);

        m_disconnected = false;
        m_player = new player::Player{ peer };

        if (!m_client) {
            m_client = new client::Client{ this };
initialize:
            if (!m_client->initialize()) {
failed_initialize:
                spdlog::error("Failed to initialize proxy client.");

                delete m_player;
                m_player = nullptr;

                delete m_client;
                m_client = nullptr;
            }
        }
        else {
            if (m_client->is_on_send_to_server()) {
                if (!m_client->create_host(1))
                    goto failed_initialize;

                if (!m_client->connect(m_client->get_host(), m_client->get_port()))
                    goto failed_initialize;

                m_client->start_service();
            }
            else {
                goto initialize;
            }
        }
    }

    void Server::on_receive(ENetPeer *peer, ENetPacket *packet)
    {
        if (!m_client)
            return;

        if (m_client->is_disconnected())
            return;

        if (!process_packet(peer, packet))
            return;

        if (m_client->get_player()->send_packet_packet(packet) != 0)
            spdlog::error("Failed to send packet to growtopia server");
    }

    void Server::on_disconnect(ENetPeer *peer)
    {
        spdlog::info("Client disconnected from Growtopia Client: (peer->data! -> {})", peer->data);

        if (m_client) {
            delete m_client;
            m_client = nullptr;
        }

        delete m_player;
        m_player = nullptr;

        m_disconnected = true;
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
                    if (text_parse.get("requestedName", 1).empty())
                        break;

                    static randutils::pcg_rng gen{ utils::random::get_generator_local() };
                    static std::string mac{ utils::random::generate_mac(gen) };
                    static std::string rid{ utils::random::generate_hex(gen, 16, true) };
                    static std::string wk{ utils::random::generate_hex(gen, 16, true) };
                    static std::string device_id{ utils::random::generate_hex(gen, 16, true) };

                    auto protocol{ Config::get().config()["server"]["protocol"].get<uint8_t>() };
                    if (text_parse.get<uint8_t>("protocol", 1) < protocol)
                        text_parse.set("protocol", protocol);

                    std::string version{ Config::get().config()["server"]["gameVersion"] };
                    version.erase(std::remove(version.begin(), version.end(), '.'), version.end());
                    if (text_parse.get<uint32_t>("game_version", 1) < std::stoul(version))
                        text_parse.set("game_version", Config::get().config()["server"]["gameVersion"]);

                    text_parse.set("mac", mac);
                    text_parse.set("rid", rid);
                    text_parse.set("wk", wk);
                    text_parse.set("hash", utils::proton_hash(fmt::format("{}RT", device_id).c_str()));
                    text_parse.set("hash2", utils::proton_hash(fmt::format("{}RT", mac).c_str()));

                    m_client->get_player()->send_packet(message_type, text_parse.get_all_raw());
                    return false;
                }
                else if (message_data.find("action|input") != std::string::npos) {
                    utils::TextParse text_parse{ message_data };
                    if (text_parse.get("text", 1).empty())
                        break;

                    if (m_command_manager->try_find_and_fire_command(this, text_parse.get("text", 1)))
                        return false;
                }
                else if (message_data.find("action|wrench") != std::string::npos) {
                    utils::TextParse text_parse{ message_data };
                    if (text_parse.get("netid", 1).empty())
                        break;

                    auto net_id = text_parse.get<uint32_t>("netid", 1);

                    player::LocalPlayer* local_player{ m_client->get_local_player() };
                    if (net_id == local_player->get_net_id())
                        break;

                    if (net_id != local_player->get_world()->world_owner_id)
                        break;

                    std::string button_clicked{};
                    if (local_player->has_flags(player::eFlag::FAST_WRENCH_PULL))
                        button_clicked = "pull";
                    else if (local_player->has_flags(player::eFlag::FAST_WRENCH_KICK))
                        button_clicked = "kick";
                    else if (local_player->has_flags(player::eFlag::FAST_WRENCH_BAN))
                        button_clicked = "worldban";

                    if (button_clicked.empty())
                        break;

                    m_client->get_player()->send_packet(
                        player::NET_MESSAGE_GENERIC_TEXT,
                        fmt::format(
                            "action|dialog_return\n"
                            "dialog_name|popup\n"
                            "netID|{}\n"
                            "buttonClicked|{}", net_id, button_clicked));
                    return false;
                }
                else if (message_data.find("action|quit") != std::string::npos &&
                    message_data.find("action|quit_to_exit") == std::string::npos) {
                    enet_peer_disconnect_now(peer, 0);

                    m_disconnected = true;
                }
                else {
                    utils::TextParse text_parse{ message_data };
                    if (text_parse.empty()) break;

                    spdlog::info(
                        "Outgoing MessagePacket:\n{} [{}]:\n{}\n",
                        player::message_type_to_string(message_type),
                        message_type,
                        fmt::join(text_parse.get_all_array(), "\r\n"));
                }
                break;
            }
            case player::NET_MESSAGE_GAME_PACKET: {
                player::GameUpdatePacket* game_update_packet{ player::get_struct(packet) };
                return process_tank_update_packet(peer, game_update_packet);
            }
            default:
                utils::TextParse text_parse{ message_data };
                if (text_parse.empty()) break;

                spdlog::info(
                    "Outgoing MessagePacket:\n{} [{}]:\n{}\n",
                    player::message_type_to_string(message_type),
                    message_type,
                    fmt::join(text_parse.get_all_array(), "\r\n"));
                break;
        }

        return true;
    }

    bool Server::process_tank_update_packet(ENetPeer* peer, player::GameUpdatePacket* game_update_packet)
    {
        switch (game_update_packet->type) {
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

                m_disconnected = true;
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
