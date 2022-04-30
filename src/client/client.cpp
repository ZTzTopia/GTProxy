#include "fmt/ranges.h"
#include <httplib.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/spdlog.h>
#include <util/Variant.h>

#include "../config.h"
#include "../server/server.h"
#include "../utils/binary_reader.h"
#include "../utils/quick_hash.h"
#include "../utils/textparse.h"
#include "../world/World.h"
#include "../world/WorldTileMap.h"
#include "client.h"

namespace client {
    Client::Client(server::Server* server)
        : enetwrapper::ENetClient(), m_proxy_server(server), m_player(nullptr), m_on_send_to_server() {}

    Client::~Client()
    {
        delete m_player;
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

        utils::TextParse text_parse{response->body};
        std::string server{ text_parse.get("server", 1) };
        enet_uint16 port{ text_parse.get<enet_uint16>("port", 1) };

        if (!connect(server, port, 1))
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
        if (!m_proxy_server || !m_proxy_server->get_player())
            return;

        player::eNetMessageType message_type{player::message_type_to_string(packet)};
        std::string message_data{ player::get_text(packet) };
        switch (message_type) {
            case player::NET_MESSAGE_GAME_PACKET: {
                player::GameUpdatePacket* game_update_packet{ player::get_struct(packet) };
                switch (game_update_packet->type) {
                    case player::PACKET_CALL_FUNCTION: {
                        uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
                        if (!extended_data) break;

                        VariantList variant_list{};
                        variant_list.SerializeFromMem(extended_data, static_cast<int>(game_update_packet->data_size));

                        std::size_t hash{ utils::quick_hash(variant_list.Get(0).GetString()) };
                        switch (hash) {
                            case "OnSpawn"_qh: {
                                utils::TextParse text_parse{ variant_list.Get(1).GetString() };
                                if (text_parse.get("type", 1) == "local") {
                                    m_player->get_avatar()->name = text_parse.get("name", 1);
                                    m_player->get_avatar()->AvatarData.net_id = text_parse.get<int32_t>("netID", 1);
                                }
                                else {
                                    auto net_id = text_parse.get<int32_t>("netID", 1);
                                    auto avatar = new NetAvatar{};
                                    avatar->name = text_parse.get("name", 1);
                                    avatar->AvatarData.net_id = net_id;
                                    m_player->get_avatar_map().insert_or_assign(net_id, avatar);
                                }
                                break;
                            }
                            case "OnSendToServer"_qh: {
                                std::vector<std::string> tokenize{
                                    utils::TextParse::string_tokenize(variant_list.Get(4).GetString()) };

                                m_on_send_to_server.active = true;
                                m_on_send_to_server.host = tokenize[0];
                                m_on_send_to_server.port = static_cast<enet_uint16>(variant_list.Get(1).GetINT32());

                                m_proxy_server->get_player()->send_variant({
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
                                return;
                            }
                            case "OnDialogRequest"_qh:
                            case "onShowCaptcha"_qh: {
                                bool is_dialog_request = hash == "OnDialogRequest"_qh;
                                bool is_captcha = hash == "onShowCaptcha"_qh;

                                std::vector<std::string> tokenize{
                                    utils::TextParse::string_tokenize(variant_list.Get(1).GetString()) };

                                for (auto& data: tokenize) {
                                    if (is_dialog_request) {
                                        auto DialogText = variant_list.Get(1).GetString();
                                        if (get_player()->get_fast_drop()) {
                                            std::string itemid = DialogText.substr(DialogText.find("embed_data|itemID|") + 18, DialogText.length() - DialogText.find("embed_data|itemID|") - 1);
                                            std::string count = DialogText.substr(DialogText.find("count||") + 7, DialogText.length() - DialogText.find("count||") - 1);
                                            if (DialogText.find("embed_data|itemID|") != -1) {
                                                if (DialogText.find("Drop") != -1) {
                                                    m_proxy_server->get_player()->send_log(fmt::format("Dropping ItemID: {}", itemid));
                                                    m_player->send_packet(player::NET_MESSAGE_GENERIC_TEXT,
                                                    fmt::format("action|dialog_return\n"
                                                        "dialog_name|drop_item\n"
                                                        "itemID|{}|\n"
                                                        "count|{}",
                                                    itemid,count));
                                                    return;
                                                }
                                            }
                                        }

                                        if (data.find("Are you Human?") != std::string::npos)
                                            is_captcha = true;
                                    }

                                    if (is_captcha && data.find(" + ") != std::string::npos) {
                                        std::vector<std::string> tokenize_{
                                            utils::TextParse::string_tokenize(data, " + ") };

                                        auto num1 = static_cast<uint8_t>(std::stoi(tokenize_[0]));
                                        auto num2 = static_cast<uint8_t>(std::stoi(tokenize_[1]));
                                        uint16_t sum = num1 + num2;

                                        m_proxy_server->get_player()->send_log(
                                            fmt::format("Captcha solver: {} + {} = {}", num1, num2, sum));
                                        m_player->send_packet(player::NET_MESSAGE_GENERIC_TEXT,
                                            fmt::format("action|dialog_return\n"
                                                "dialog_name|captcha_submit\n"
                                                "captcha_answer|{}",
                                                sum));
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

                        World* world = m_player->get_world();
                        world->serialize(extended_data);

                        m_player->get_avatar()->world_name = world->name;
                        break;
                    }
                    case player::PACKET_SEND_TILE_UPDATE_DATA: {
                        uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
                        if (!extended_data) break;

                        Tile tile{};
                        BinaryReader br{ extended_data };

                        // TODO: Move to Tile::serialize()
                        tile.foreground = br.read_u16();
                        tile.background = br.read_u16();
                        tile.parent_tile = br.read_u16();
                        tile.flags = static_cast<Tile::TileFlag>(br.read_u16());

                        if (tile.parent_tile) br.skip(2);
                        if (!(tile.flags & Tile::EXTRA)) break;

                        TileExtra* tile_extra{ tile.tile_extra };

                        // TODO: Move to TileExtra::serialize()
                        tile_extra->m_type = static_cast<TileExtra::ExtraType>(br.read_u8());
                        if (tile_extra->m_type == TileExtra::TYPE_NONE) break;

                        switch (tile_extra->m_type) {
                            case TileExtra::TYPE_DOOR:
                                tile_extra->m_door.label = br.read_string();
                                tile_extra->m_door.unk = br.read_u8();
                                break;
                            case TileExtra::TYPE_SIGN:
                                tile_extra->m_sign.label = br.read_string();
                                tile_extra->m_sign.unk = br.read_u32();
                                break;
                            case TileExtra::TYPE_SEED:
                                tile_extra->m_seed.growth_time = br.read_u32();
                                tile_extra->m_seed.fruit_count = br.read_u8();
                                break;
                            case TileExtra::TYPE_PROVIDER: {
                                tile_extra->m_provider.unk = br.read_u32();

                                World* world = m_player->get_world();
                                if (tile.foreground != 5318 && (tile.foreground != 10656 || world->version < 17) )
                                    break;

                                tile_extra->m_provider.unk2 = br.read_u32();
                                break;
                            }
                            case TileExtra::TYPE_HEART_MONITOR:
                                tile_extra->m_heart_monitor.unk = br.read_u32();
                                tile_extra->m_heart_monitor.label = br.read_string();
                                break;
                            default:
                                break;
                        }

                        spdlog::info(
                            "Incoming PACKET_SEND_TILE_UPDATE_DATA\n > TileExtra_Type -> [{}]:\n{}",
                            TileExtra::GetTypeAsString(tile_extra->m_type), tile_extra->GetRawData());
                        break;
                    }
                    case player::PACKET_SET_CHARACTER_STATE: {
                        if (game_update_packet->net_id == m_player->get_avatar()->AvatarData.net_id) {
                            std::memcpy(&m_player->get_avatar()->AvatarData, packet->data + 4, packet->dataLength - 4);
                            break;
                        }
                        else {
                            for (auto& avatar: m_player->get_avatar_map()) {
                                if (avatar.first == game_update_packet->net_id) {
                                    std::memcpy(&avatar.second->AvatarData, packet->data + 4, packet->dataLength - 4);
                                    break;
                                }
                            }
                        }
                        break;
                    }
                    case player::PACKET_SEND_INVENTORY_STATE: {
                        uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
                        if (!extended_data) break;

                        PlayerItems* inventory = m_player->get_inventory();
                        inventory->serialize(extended_data);
                        break;
                    }
                    case player::PACKET_MODIFY_ITEM_INVENTORY: {
                        PlayerItems* inventory = m_player->get_inventory();

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

        if (m_proxy_server->get_player()->send_packet_packet(packet) != 0)
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

        if (m_proxy_server->get_player())
            enet_peer_disconnect_now(m_proxy_server->get_player()->get_peer(), 0);

        m_proxy_server = nullptr;
    }
}// namespace client
