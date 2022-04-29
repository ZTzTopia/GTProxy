#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <util/Variant.h>
#include <httplib.h>
#include "fmt/ranges.h"

#include "client.h"
#include "../config.h"
#include "../server/server.h"
#include "../world/WorldTileMap.h"
#include "../utils/binary_reader.h"
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
        if (!m_proxy_server || !m_proxy_server->get_player() || !m_player)
            return;

        m_send_server_info->check = false;

        player::eNetMessageType message_type{ player::get_message_type(packet) };
        std::string message_data{ player::get_text(packet) };
        switch (message_type) {
            case player::NET_MESSAGE_GAME_PACKET: {
                player::GameUpdatePacket *updatePacket{player::get_struct(packet)};
                switch (updatePacket->type) {
                    case player::PACKET_CALL_FUNCTION: {
                        uint8_t* extended_data{ player::get_extended_data(updatePacket) };
                        if (!extended_data)
                            break;

                        VariantList variant_list{};
                        variant_list.SerializeFromMem(extended_data, static_cast<int>(updatePacket->data_size));

                        const auto& hash = utils::quick_hash(variant_list.Get(0).GetString());
                        switch (hash) {
                            case "OnSpawn"_qh: {
                                utils::TextParse text_parse{ variant_list.Get(1).GetString() };
                                if(text_parse.get("type", 1) == "local") {
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

                                spdlog::info("Send to {}:{}", m_send_server_info->host, m_send_server_info->port);
                                return;
                            }
                            case "onShowCaptcha"_qh: {
                                std::vector<std::string> tokenize{ utils::TextParse::string_tokenize(variant_list.Get(1).GetString()) };
                                for (auto &data : tokenize) {
                                    if (data.find(" + ") != std::string::npos) {
                                        std::vector<std::string> tokenize_{ utils::TextParse::string_tokenize(data, " + ") };
                                        auto sum_1 = static_cast<uint8_t>(std::stoi(tokenize_[0]));
                                        auto sum_2 = static_cast<uint8_t>(std::stoi(tokenize_[1]));
                                        auto sum = sum_1 + sum_2;
                                        m_proxy_server->get_player()->send_log(fmt::format("Solved captcha: {} + {} = {}", sum_1, sum_2, sum));
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
                        uint8_t* extended_data{ player::get_extended_data(updatePacket) };
                        if (!extended_data)
                            break;

                        BinaryReader br{ extended_data + 6, updatePacket->data_size + 6 };
                        m_player->get_avatar()->world_name = br.read_string();
                        break;
                    }
                    case player::PACKET_SEND_TILE_UPDATE_DATA: {
                        uint8_t* extended_data{ player::get_extended_data(updatePacket) };
                        if (!extended_data || updatePacket->data_size < 8)
                            break;

                        BinaryReader br{ extended_data, updatePacket->data_size };
                        Tile tile{};
                        br.copy(&tile, sizeof(uint16_t) * 4);
                        if (tile.parent_tile)
                            br.skip(2);

                        if (!(tile.flags & Tile::EXTRA_DATA)) {
                            break;
                        }

                        TileExtra* tile_extra{ tile.tile_extra };
                        tile_extra->type = (TileExtra::ExtraType)br.read_byte();
                        switch(tile_extra->type) {
                            case TileExtra::TYPE_DOOR: {
                                tile_extra->label = br.read_string();
                                tile_extra->unk_1 = br.read_byte();
                                break;
                            }
                            case TileExtra::TYPE_SIGN: {
                                tile_extra->label = br.read_string();
                                tile_extra->unk_2 = br.read_uint();
                                break;
                            }
                            case TileExtra::TYPE_LOCK: {
                                tile_extra->flags_1 = br.read_byte();
                                tile_extra->user_id = br.read_uint();
                                switch(tile.foreground) {
                                    case 202:
                                    case 204:
                                    case 206:
                                    case 4994: {
                                        const int& access_count = br.read_int();
                                        for(int i = 0; i < access_count; i++)
                                            tile_extra->access_list.push_back(br.read_uint());
                                        break;
                                    }
                                    default: {
                                        break;
                                    }
                                }
                                br.copy(tile_extra->unk7a, 8);
                                break;
                            }
                            default:
                                break;
                        }

                        if (tile_extra->type != TileExtra::TYPE_NONE)
                            spdlog::info("Incoming PACKET_SEND_TILE_UPDATE_DATA\n > TileExtra_Type -> [{}]:\n{}", TileExtra::GetTypeAsString(tile_extra->type), tile_extra->GetRawData());
                        break;
                    }
                    case player::PACKET_SET_CHARACTER_STATE: {
                        if(updatePacket->net_id == m_player->get_avatar()->AvatarData.net_id) {
                            std::memcpy(&m_player->get_avatar()->AvatarData, packet->data + 4, packet->dataLength - 4);
                            break;
                        }
                        else {
                            for (auto& avatar : m_player->get_avatar_map()) {
                                if (avatar.first == updatePacket->net_id) {
                                    std::memcpy(&avatar.second->AvatarData, packet->data + 4, packet->dataLength - 4);
                                    break;
                                }
                            }
                        }
                        break;
                    }
                    case player::PACKET_SEND_INVENTORY_STATE: {
                        uint8_t* extended_data{ player::get_extended_data(updatePacket) };
                        if (!extended_data)
                            break;

                        PlayerItems* inventory = m_player->get_inventory();
                        inventory->items.clear();

                        BinaryReader binary_reader{ extended_data, updatePacket->data_size };
                        inventory->version = binary_reader.read_byte();
                        inventory->max_size = binary_reader.read_uint();

                        if (inventory->version >= 1)
                            inventory->size = binary_reader.read_ushort();
                        else
                            inventory->size = binary_reader.read_byte();

                        for (uint32_t i = 0; i < inventory->size; i++) {
                            uint16_t id = binary_reader.read_ushort();
                            uint8_t count = binary_reader.read_byte();
                            uint8_t unused = binary_reader.read_byte();
                            inventory->items.insert(std::make_pair(id, std::make_pair(count, unused)));
                        }
                        break;
                    }
                    case player::PACKET_MODIFY_ITEM_INVENTORY: {
                        PlayerItems* inventory = m_player->get_inventory();

                        bool found{ false };
                        for (auto& item : inventory->items) {
                            if (item.first == updatePacket->item_id) {
                                if (updatePacket->gained_item_count > 0)
                                    item.second.first += updatePacket->gained_item_count;
                                else if (updatePacket->lost_item_count > 0)
                                    item.second.first -= updatePacket->lost_item_count;

                                if (item.second.first <= 0)
                                    inventory->items.erase(item.first);

                                found = true;
                                break;
                            }
                        }

                        if (!found && updatePacket->gained_item_count > 0)
                            inventory->items.insert(std::make_pair(updatePacket->item_id, std::make_pair(updatePacket->gained_item_count, 0)));
                        break;
                    }
                    case player::PACKET_APP_INTEGRITY_FAIL:
                        return;
                    default: {
                        uint8_t *extended_data{ player::get_extended_data(updatePacket) };
                        std::vector<char> data_array;
                        for (uint32_t i = 0; i < updatePacket->data_size; i++)
                            data_array.push_back(static_cast<char>(extended_data[i]));
                        spdlog::info("Incoming GameUpdatePacket:\n [{}]{}{}", 
                            updatePacket->type, 
                            player::get_packet_type(updatePacket->type),
                            extended_data ? fmt::format("\n > extended_data: {}", spdlog::to_hex(data_array)) : "");
                        break;
                    }
                }
                break;
            }
            default: {
                utils::TextParse text_parse{ message_data };
                if (text_parse.empty())
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

        enet_host_flush(m_host);
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
