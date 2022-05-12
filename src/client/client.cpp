#include "fmt/ranges.h"
#include <httplib.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/spdlog.h>
#include <util/Variant.h>
#include <algorithm>

#include "../config.h"
#include "../server/server.h"
#include "../utils/binary_reader.h"
#include "../utils/hash.h"
#include "../utils/textparse.h"
#include "../world/world.h"
#include "../world/world_tile_map.h"
#include "client.h"

namespace client {
    Client::Client(server::Server* server)
        : enetwrapper::ENetClient(), m_server(server), m_player(nullptr), m_remote_player(), m_on_send_to_server() {
        m_local_player = new player::LocalPlayer{};
    }

    Client::~Client()
    {
        delete m_player;
        delete m_local_player;

        for (auto& pair : m_remote_player) {
            delete pair.second;
        }

        m_remote_player.clear();
    }

    bool Client::initialize()
    {
        // Get server and port from server_data.php.
        httplib::Client http_client{ Config::get().config()["server"]["host"] };
        httplib::Result response = http_client.Post("/growtopia/server_data.php");
        if (response.error() != httplib::Error::Success || response->status != 200) {
            spdlog::error("Failed to get server data. HTTP status code: {}", response->status);
            return false;
        }

        utils::TextParse text_parse{ response->body };
        std::string server{ text_parse.get("server", 1) };
        enet_uint16 port{ text_parse.get<enet_uint16>("port", 1) };

        if (!create_host(1))
            return false;

        if (!connect(server, port))
            return false;

        start_service();
        return true;
    }

    void Client::on_connect(ENetPeer* peer)
    {
        spdlog::info("Client connected to growtopia server: {}", peer->connectID);
        m_on_send_to_server.active = false;
        m_player = new player::Player{ peer };
    }

    void Client::on_receive(ENetPeer* peer, ENetPacket* packet)
    {
        if (!m_player) return;
        if (!m_server || !m_server->get_player())
            return;

        player::eNetMessageType message_type{player::message_type_to_string(packet)};
        std::string message_data{ player::get_text(packet) };
        switch (message_type) {
            case player::NET_MESSAGE_GAME_PACKET: {
                player::GameUpdatePacket* game_update_packet{ player::get_struct(packet) };
                switch (game_update_packet->type) {
                    // TODO: Create new methode or file to handle these packets.
                    case player::PACKET_CALL_FUNCTION: {
                        uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
                        if (!extended_data) break;

                        VariantList variant_list{};
                        variant_list.SerializeFromMem(extended_data, static_cast<int>(game_update_packet->data_size));

                        std::size_t hash{ utils::fnv1a_hash(variant_list.Get(0).GetString()) };
                        switch (hash) {
                            case "OnRequestWorldSelectMenu"_fh: {
                                for (auto& pair : m_remote_player) {
                                    delete pair.second;
                                    pair.second = nullptr;
                                }

                                m_remote_player.clear();
                                break;
                            }
                            case "OnSpawn"_fh: {
                                utils::TextParse text_parse{ variant_list.Get(1).GetString() };

                                auto net_id = text_parse.get<uint32_t>("netID", 1);

                                if (text_parse.get("type", 1) == "local") {
                                    m_local_player->set_net_id(net_id);
                                }
                                else {
                                    m_remote_player[net_id] = new player::RemotePlayer{};
                                    m_remote_player[net_id]->set_net_id(net_id);
                                }
                                break;
                            }
                            case "OnRemove"_fh: {
                                utils::TextParse text_parse{ variant_list.Get(1).GetString() };

                                auto net_id = text_parse.get<uint32_t>("netID", 1);

                                delete m_remote_player[net_id];
                                m_remote_player[net_id] = nullptr;
                                break;
                            }
                            case "OnSendToServer"_fh: {
                                std::vector<std::string> tokenize{
                                    utils::TextParse::string_tokenize(variant_list.Get(4).GetString()) };

                                m_on_send_to_server.active = true;
                                m_on_send_to_server.host = tokenize[0];
                                m_on_send_to_server.port = static_cast<enet_uint16>(variant_list.Get(1).GetINT32());

                                m_server->get_player()->send_variant({
                                    "OnSendToServer",
                                    17000,
                                    variant_list.Get(2).GetINT32(),
                                    variant_list.Get(3).GetINT32(),
                                    fmt::format(
                                        "127.0.0.1|{}|{}", tokenize.size() == 2 ? "" : tokenize.at(1)
                                        , tokenize.size() == 2 ? tokenize.at(1) : tokenize.at(2)),
                                    variant_list.Get(5).GetINT32() });

                                spdlog::info(
                                    "OnSendToServer: {}:{}",
                                    m_on_send_to_server.host,
                                    m_on_send_to_server.port);

                                enet_host_flush(m_host);
                                return;
                            }
                            case "OnDialogRequest"_fh:
                            case "onShowCaptcha"_fh: {
                                bool is_dialog_request = hash == "OnDialogRequest"_fh;
                                bool is_captcha = hash == "onShowCaptcha"_fh;

                                if (is_dialog_request) {
                                    utils::TextParse text_parse{ variant_list.Get(1).GetString() };

                                    if (variant_list.Get(1).GetString().find("drop_item") != std::string::npos) {
                                        if (!m_local_player->has_flags(player::eFlag::FAST_DROP))
                                            break;

                                        uint8_t count{ text_parse.get<uint8_t>("add_text_input", 2) };
                                        uint16_t item_id{ text_parse.get<uint8_t>("embed_data", 2) };

                                        m_server->get_player()->send_log(fmt::format("You dropping item id: {}", item_id));
                                        m_player->send_packet(
                                            player::NET_MESSAGE_GENERIC_TEXT,
                                            fmt::format(
                                                "action|dialog_return\n"
                                                "dialog_name|drop_item\n"
                                                "itemID|{}\n"
                                                "count|{}",
                                                item_id, count));

                                        enet_host_flush(m_host);
                                        return;
                                    }
                                }

                                std::vector<std::string> tokenize{
                                    utils::TextParse::string_tokenize(variant_list.Get(1).GetString()) };

                                for (auto& data: tokenize) {
                                    if (is_dialog_request) {
                                        if (data.find("Are you Human?") != std::string::npos)
                                            is_captcha = true;
                                    }

                                    if (is_captcha && data.find(" + ") != std::string::npos) {
                                        std::vector<std::string> tokenize_{
                                            utils::TextParse::string_tokenize(data, " + ") };

                                        auto num1 = static_cast<uint8_t>(std::stoi(tokenize_[0]));
                                        auto num2 = static_cast<uint8_t>(std::stoi(tokenize_[1]));
                                        uint16_t sum = num1 + num2;

                                        m_server->get_player()->send_log(
                                            fmt::format("Solved captcha: {} + {} = {}", num1, num2, sum));
                                        m_player->send_packet(player::NET_MESSAGE_GENERIC_TEXT,
                                            fmt::format("action|dialog_return\n"
                                                "dialog_name|captcha_submit\n"
                                                "captcha_answer|{}",
                                                sum));

                                        enet_host_flush(m_host);
                                        return;
                                    }
                                }
                                break;
                            }
                            default: {
                                spdlog::info("Incoming VariantList:\n{}", variant_list.GetContentsAsDebugString());
                                break;
                            }
                        }
                        break;
                    }
                    case player::PACKET_SEND_MAP_DATA: {
                        uint8_t* extended_data{player::get_extended_data(game_update_packet)};
                        if (!extended_data) break;

                        World* world = m_local_player->get_world();
                        world->serialize(extended_data);
                        break;
                    }
                    case player::PACKET_SEND_TILE_UPDATE_DATA: {
                        uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
                        if (!extended_data) break;

                        Tile tile{};
                        tile.serialize(extended_data, m_local_player->get_world()->version);

                        spdlog::info("Incoming PACKET_SEND_TILE_UPDATE_DATA\n{}", tile.get_raw_data());
                        break;
                    }
                    case player::PACKET_SEND_INVENTORY_STATE: {
                        uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
                        if (!extended_data) break;

                        PlayerItems* inventory = m_local_player->get_items();
                        inventory->serialize(extended_data);
                        break;
                    }
                    case player::PACKET_MODIFY_ITEM_INVENTORY: {
                        PlayerItems* inventory = m_local_player->get_items();

                        bool found{ false };
                        for (auto& item: inventory->items) {
                            if (item.first == game_update_packet->item_id) {
                                if (game_update_packet->gained_item_count > 0)
                                    item.second.first += game_update_packet->gained_item_count;
                                else if (game_update_packet->lost_item_count > 0)
                                    item.second.first -= game_update_packet->lost_item_count;

                                if (item.second.first <= 0)
                                    inventory->items.erase(item.first);

                                found = true;
                                break;
                            }
                        }

                        if (!found && game_update_packet->gained_item_count > 0)
                            inventory->items.insert(
                                std::make_pair(game_update_packet->item_id,
                                std::make_pair(game_update_packet->gained_item_count, 0)));
                        break;
                    }
                    case player::PACKET_ITEM_CHANGE_OBJECT: {
                        World* world = m_local_player->get_world();

                        if (game_update_packet->object_change_type == -1) {
                            world->object_map.count++;
                            world->object_map.drop_id++;

                            Object object{};
                            object.pos = { game_update_packet->pos_x, game_update_packet->pos_y };
                            object.item_id = game_update_packet->item_id;
                            object.amount = static_cast<uint8_t>(game_update_packet->obj_alt_count);
                            object.drop_id_offset = world->object_map.drop_id;
                            world->object_map.objects.push_back(object);
                        }

                        for (auto& object : world->object_map.objects) {
                            if (object.drop_id_offset == game_update_packet->item_net_id) {
                                if (game_update_packet->object_change_type == -3) {
                                    object.pos = { game_update_packet->pos_x, game_update_packet->pos_y };
                                    object.item_id = game_update_packet->item_id;
                                    object.amount = static_cast<uint8_t>(game_update_packet->obj_alt_count);
                                }
                            }
                        }

                        if (game_update_packet->object_change_type != -1 && game_update_packet->object_change_type != -3) {
                            for (auto it = world->object_map.objects.begin(); it != world->object_map.objects.end();) {
                                if (it->drop_id_offset == game_update_packet->object_id) {
                                    it = world->object_map.objects.erase(it);
                                    world->object_map.count--;
                                }
                            }
                        }
                        break;
                    }
                    case player::PACKET_APP_INTEGRITY_FAIL:
                        return;
                    default: {
                        uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
                        
                        std::vector<char> data_array;
                        for (uint32_t i = 0; i < game_update_packet->data_size; i++)
                            data_array.push_back(static_cast<char>(extended_data[i]));
                        
                        spdlog::info(
                            "Incoming TankUpdatePacket:\n [{}]{}{}",
                            game_update_packet->type,
                            player::packet_type_to_string(game_update_packet->type),
                            extended_data ? fmt::format("\n > extended_data: {}", spdlog::to_hex(data_array)) : "");
                        break;
                    }
                }
                break;
            }
            default: {
                utils::TextParse text_parse{message_data};
                if (text_parse.empty()) break;

                spdlog::info(
                    "Incoming MessagePacket:\n{} [{}]:\n{}\n",
                    player::message_type_to_string(message_type),
                    message_type,
                    fmt::join(text_parse.get_all_array(), "\r\n"));
                break;
            }
        }

        if (m_server->get_player()->send_packet_packet(packet) != 0)
            spdlog::error("Failed to send packet to growtopia client!");

        enet_host_flush(m_host);
    }

    void Client::on_disconnect(ENetPeer* peer)
    {
        spdlog::info("Client disconnected from Growtopia Server: (peer->data! -> {})", peer->data);

        if (!m_player) return;
        if (m_player->get_peer())
            enet_peer_disconnect_now(m_player->get_peer(), 0);

        delete m_player;
        m_player = nullptr;

        if (m_server->get_player())
            enet_peer_disconnect_now(m_server->get_player()->get_peer(), 0);

        m_server = nullptr;
    }
}// namespace client
