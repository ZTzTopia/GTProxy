#include <algorithm>
#include <fmt/ranges.h>
#include <httplib.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/spdlog.h>
#include <util/Variant.h>
#include <util/ResourceUtils.h>

#include "client.h"
#include "../config.h"
#include "../server/server.h"
#include "../utils/hash.h"
#include "../utils/text_parse.h"

namespace client {
    Client::Client(server::Server* server)
        : enetwrapper::ENetClient(), m_server(server), m_player(nullptr), m_remote_player(), m_on_send_to_server()
        , m_disconnected(true)
    {
        m_local_player = new player::LocalPlayer{ m_player };
        m_items = new items::Items{};

        m_on_update_thread_running.store(true);
        std::thread th{ &Client::on_update, this };
        m_on_update_thread = std::move(th);
    }

    Client::~Client()
    {
        m_on_update_thread_running.store(false);
        m_on_update_thread.join();

        delete m_player;

        for (auto& item_info : m_items->items) {
            delete item_info;
        }

        delete m_items;
        delete m_local_player;

        for (auto& pair : m_remote_player) {
            delete pair.second;
        }

        m_remote_player.clear();
    }

    bool Client::initialize()
    {
        // Get server and port from server_data.php.
        httplib::Client cli{ Config::get().config()["server"]["host"] };
        httplib::Result response{ cli.Post("/growtopia/server_data.php") };
        if (response.error() != httplib::Error::Success || response->status != 200) {
            if (response.error() == httplib::Error::Success)
                spdlog::error("Failed to get server data. HTTP status code: {}", response->status);
            else
                spdlog::error("Failed to get server data. HTTP error: {}", httplib::to_string(response.error()));

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

    void Client::on_update()
    {
        while (m_on_update_thread_running.load()) {
            if (!m_local_player) continue;

            World* world = m_local_player->get_world();
            if (!world) continue;

            m_local_player->on_update(this, m_items);

            // 20 ticks per second (called every 0.05 seconds).
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    void Client::on_connect(ENetPeer* peer)
    {
        spdlog::info("Client connected to growtopia server: {}", peer->connectID);
        m_disconnected = false;
        m_on_send_to_server.active = false;
        m_player = new player::Player{ peer };
    }

    void Client::on_receive(ENetPeer* peer, ENetPacket* packet)
    {
        if (!m_server)
            return;

        if (m_server->is_disconnected())
            return;

        if (!process_packet(peer, packet))
            return;

        if (m_server->get_player()->send_packet_packet(packet) != 0)
            spdlog::error("Failed to send packet to growtopia client!");
    }

    void Client::on_disconnect(ENetPeer* peer)
    {
        spdlog::info("Client disconnected from Growtopia Server: (peer->data! -> {})", peer->data);

        delete m_player;
        m_player = nullptr;

        m_disconnected = true;
    }

    bool Client::process_packet(ENetPeer* peer, ENetPacket* packet)
    {
        player::eNetMessageType message_type{player::message_type_to_string(packet)};
        std::string message_data{ player::get_text(packet) };
        switch (message_type) {
            case player::NET_MESSAGE_GAME_PACKET: {
                player::GameUpdatePacket* game_update_packet{ player::get_struct(packet) };
                return process_tank_update_packet(peer, game_update_packet);
            }
            case player::NET_MESSAGE_TRACK: {
                utils::TextParse text_parse{ message_data };
                uint8_t event_type{ text_parse.get<uint8_t>("eventType", 1) };
                if (event_type == 0) {
                    std::string event_name{ text_parse.get("eventName", 1) };
                    if (event_name == "300_WORLD_VISIT") {
                        uint32_t world_owner{ text_parse.get<uint32_t>("World_owner", 1) };
                        if (world_owner != 0)
                            m_local_player->get_world()->world_owner_id = world_owner;
                    }
                }
            }
            default: {
                utils::TextParse text_parse{ message_data };
                if (text_parse.empty()) break;

                spdlog::info(
                    "Incoming MessagePacket:\n{} [{}]:\n{}\n",
                    player::message_type_to_string(message_type),
                    message_type,
                    fmt::join(text_parse.get_all_array(), "\r\n"));
                break;
            }
        }

        return true;
    }

    bool Client::process_tank_update_packet(ENetPeer* peer, player::GameUpdatePacket* game_update_packet)
    {
        switch (game_update_packet->type) {
            case player::PACKET_STATE:
                break;
            case player::PACKET_CALL_FUNCTION: {
                uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
                if (!extended_data) break;

                VariantList variant_list{};
                variant_list.SerializeFromMem(extended_data, static_cast<int>(game_update_packet->data_size));

                spdlog::info("Incoming VariantList:\n{}", variant_list.GetContentsAsDebugString());

                std::size_t hash{ utils::fnv1a_hash(variant_list.Get(0).GetString()) };
                switch (hash) {
                    case "OnRequestWorldSelectMenu"_fh: {
                        m_local_player->get_items()->items.clear();

                        for (auto& tile : m_local_player->get_world()->tile_map.tiles) {
                            delete tile;
                        }

                        m_local_player->get_world()->tile_map.tiles.clear();

                        for (auto& object : m_local_player->get_world()->object_map.objects) {
                            delete object;
                        }

                        m_local_player->get_world()->object_map.objects.clear();

                        for (auto& pair : m_remote_player) {
                            delete pair.second;
                            pair.second = nullptr;
                        }

                        m_remote_player.clear();
                        break;
                    }
                    case "OnSpawn"_fh: {
                        utils::TextParse text_parse{ variant_list.Get(1).GetString() };

                        auto net_id{ text_parse.get<uint32_t>("netID", 1) };
                        auto user_id{ text_parse.get<uint32_t>("userID", 1) };
                        auto display_name{ text_parse.get("name", 1) };
                        auto raw_name{ text_parse.get("name", 1) };
                        auto invis{ text_parse.get<uint8_t>("invis", 1) };
                        auto mod_state{ text_parse.get<uint8_t>("mstate", 1) };
                        auto supermod_state{ text_parse.get<uint8_t>("smstate", 1) };

                        bool prev_is_backtick = false;
                        raw_name.erase(std::remove_if(raw_name.begin(), raw_name.end(),
                            [&prev_is_backtick](unsigned char curr) {
                                bool r = curr == '`' || prev_is_backtick;
                                prev_is_backtick = curr == '`';
                                return r;
                            }), raw_name.end());

                        if (text_parse.get("type", 1) == "local") {
                            m_local_player->set_net_id(net_id);
                            m_local_player->set_user_id(user_id);

                            text_parse.set("mstate", "1");
                            variant_list.Get(1).Set(text_parse.get_all_raw());

                            uint32_t size;
                            uint8_t* data = variant_list.SerializeToMem(&size, nullptr);

                            game_update_packet->data_size = size;

                            m_server->get_player()->send_raw_packet(player::NET_MESSAGE_GAME_PACKET, game_update_packet,
                                sizeof(player::GameUpdatePacket), data);
                            return false;
                        }
                        else {
                            m_remote_player[net_id] = new player::RemotePlayer{};
                            m_remote_player[net_id]->set_net_id(net_id);

                            if (invis == 1 || mod_state == 1 || supermod_state == 1) {
                                m_server->get_player()->send_log("`4A moderator enters the world!");
                            }
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
                        return false;
                    }
                    case "OnDialogRequest"_fh: {
                        utils::TextParse text_parse{ variant_list.Get(1).GetString() };
                        if (variant_list.Get(1).GetString().find("drop_item") != std::string::npos) {
                            if (!m_local_player->has_flags(player::eFlag::FAST_DROP))
                                break;

                            uint8_t count{ text_parse.get<uint8_t>("add_text_input", 2) };
                            uint16_t item_id{ text_parse.get<uint16_t>("embed_data", 2) };

                            m_server->get_player()->send_log(fmt::format("You dropped item id: {}", item_id));
                            m_player->send_packet(
                                player::NET_MESSAGE_GENERIC_TEXT,
                                fmt::format(
                                    "action|dialog_return\n"
                                    "dialog_name|drop_item\n"
                                    "itemID|{}\n"
                                    "count|{}",
                                    item_id, count));
                            return false;
                        }
                        if (variant_list.Get(1).GetString().find("trash_item") != std::string::npos) {
                            if (!m_local_player->has_flags(player::eFlag::FAST_TRASH))
                                break;

                            uint8_t count{ 1 };
                            uint16_t item_id{ text_parse.get<uint16_t>("embed_data", 2) };

                            PlayerItems* inventory = m_local_player->get_items();
                            for (auto& item: inventory->items) {
                                if (item.first == item_id)
                                    count = item.second.first;
                            }

                            m_server->get_player()->send_log(fmt::format("You trashed item id: {}", item_id));
                            m_player->send_packet(
                                player::NET_MESSAGE_GENERIC_TEXT,
                                fmt::format(
                                    "action|dialog_return\n"
                                    "dialog_name|trash_item\n"
                                    "itemID|{}\n"
                                    "count|{}",
                                    item_id, count));
                            return false;
                        }
                        break;
                    }
                    case "OnSuperMainStartAcceptLogonHrdxs47254722215a"_fh: {
                        if (m_items->version != 0)
                            break;

                        std::ifstream items_dat{ "items.dat", std::ofstream::binary };
                        if (!items_dat.is_open()) {
                            m_player->send_packet(
                                player::NET_MESSAGE_GENERIC_TEXT,
                                "action|refresh_item_data\n");
                            break;
                        }

                        std::streampos begin, end;
                        begin = items_dat.tellg();
                        items_dat.seekg(0, std::ios::end);
                        end = items_dat.tellg();
                        items_dat.seekg(0, std::ios::beg);

                        auto size = static_cast<std::size_t>(end - begin);

                        char* data = new char[size];
                        items_dat.read(data, static_cast<std::streamsize>(size));
                        items_dat.close();

                        if (utils::proton_hash(data, size) != variant_list.Get(1).GetUINT32()) {
                            m_player->send_packet(
                                player::NET_MESSAGE_GENERIC_TEXT,
                                "action|refresh_item_data\n");
                        }
                        else {
                            m_items->serialize(data);
                        }

                        delete[] data;
                        break;
                    }
                    case "onShowCaptcha"_fh: {
                        utils::TextParse text_parse{ variant_list.Get(1).GetString() };
                        if (text_parse.get("add_puzzle_captcha", 1).empty())
                            break;

                        std::string captcha_uuid{ text_parse.get("add_puzzle_captcha", 1) };
                        captcha_uuid.erase(0, captcha_uuid.find("0098/captcha/generated/") + 23);
                        captcha_uuid.erase(captcha_uuid.find("-PuzzleWithMissingPiece.rttex"), std::string::npos);

                        httplib::Client cli{ "puzzlecaptchasolverv2.herokuapp.com" };

                        httplib::Params params;
                        params.emplace("type", "puzzlecaptchasolver");
                        params.emplace("uuid", captcha_uuid);

                        httplib::Headers headers;
                        headers.emplace(
                            "User-Agent",
                            "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/104.0.5095.0 Safari/537.36");

try_request_again:
                        httplib::Result response{ cli.Get("/api", params, headers) };
                        if (response.error() != httplib::Error::Success || response->status != 200) {
                            if (response.error() == httplib::Error::Success)
                                spdlog::error("Failed to get server data. HTTP status code: {}", response->status);
                            else
                                spdlog::error("Failed to get server data. HTTP error: {}", httplib::to_string(response.error()));

                            if (static_cast<int>(response.error()) == 4)
                                goto try_request_again;

                            return false;
                        }

                        if (response->body != "failed") {
                            m_player->send_packet(
                                player::NET_MESSAGE_GENERIC_TEXT,
                                fmt::format(
                                    "action|dialog_return\n"
                                    "dialog_name|puzzle_captcha_submit\n"
                                    "captcha_answer|{}|CaptchaID|{}", response->body, captcha_uuid));
                            return false;
                        }
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case player::PACKET_TILE_CHANGE_REQUEST: {
                World* world = m_local_player->get_world();
                uint32_t index{ game_update_packet->int_x + game_update_packet->int_y * world->tile_map.size.x };

                if (game_update_packet->item_id == 18) {
                    if (world->tile_map.tiles[index]->foreground != 0) {
                        world->tile_map.tiles[index]->foreground = 0;
                    }
                    else {
                        world->tile_map.tiles[index]->background = 0;
                    }
                }

                if (m_items->get_item(game_update_packet->item_id)->action_type == 17) {
                    world->tile_map.tiles[index]->foreground = game_update_packet->item_id;
                }
                else if (m_items->get_item(game_update_packet->item_id)->action_type == 18) {
                    world->tile_map.tiles[index]->background = game_update_packet->item_id;
                }
                break;
            }
            case player::PACKET_SEND_MAP_DATA: {
                uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
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
                if (game_update_packet->object_change_type > 0) {
                    // TODO: Implement
                    break;
                }

                World* world = m_local_player->get_world();

                if (game_update_packet->object_change_type == -1) {
                    world->object_map.count++;
                    world->object_map.drop_id++;

                    Object object{};
                    object.pos = { game_update_packet->pos_x, game_update_packet->pos_y };
                    object.item_id = game_update_packet->item_id;
                    object.amount = static_cast<uint8_t>(game_update_packet->obj_alt_count);
                    object.drop_id_offset = world->object_map.drop_id;
                    world->object_map.objects.push_back(new Object{ object });
                }

                for (auto& object : world->object_map.objects) {
                    if (object->drop_id_offset == game_update_packet->item_net_id) {
                        if (game_update_packet->object_change_type == -3) {
                            object->pos = { game_update_packet->pos_x, game_update_packet->pos_y };
                            object->item_id = game_update_packet->item_id;
                            object->amount = static_cast<uint8_t>(game_update_packet->obj_alt_count);
                        }
                    }
                }

                if (game_update_packet->object_change_type != -1 && game_update_packet->object_change_type != -3) {
                    for (auto& object : world->object_map.objects) {
                        if (object->drop_id_offset == game_update_packet->object_id) {
                            world->object_map.objects.erase(std::remove(world->object_map.objects.begin(),
                                world->object_map.objects.end(), object), world->object_map.objects.end());
                            world->object_map.count--;

                            PlayerItems* inventory = m_local_player->get_items();

                            bool found{ false };
                            for (auto& item: inventory->items) {
                                if (item.first == object->item_id) {
                                    item.second.first += object->amount;
                                    found = true;
                                    break;
                                }
                            }

                            if (!found) {
                                inventory->items.insert(
                                    std::make_pair(object->item_id,
                                    std::make_pair(object->amount, 0)));
                            }

                            delete object;
                            break;
                        }
                    }
                }
                break;
            }
            case player::PACKET_SEND_ITEM_DATABASE_DATA: {
                std::ofstream items_dat{ "items.dat", std::ofstream::binary };
                if (items_dat.is_open()) {
                    uint8_t* data = zLibInflateToMemory(
                        player::get_extended_data(game_update_packet),
                        game_update_packet->data_size,
                        game_update_packet->dec_item_data_size);

                    m_items->serialize(data);

                    items_dat.write(reinterpret_cast<char *>(data), game_update_packet->dec_item_data_size);
                    items_dat.close();
                    delete[] data;
                }
                break;
            }
            case player::PACKET_APP_INTEGRITY_FAIL:
                return false;
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

        return true;
    }
}// namespace client
