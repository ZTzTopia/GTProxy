#include <magic_enum.hpp>
#include <openssl/evp.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>

#include "server.h"
#include "../client/client.h"
#include "../utils/hash.h"
#include "../utils/random.h"
#include "../utils/text_parse.h"

namespace server {
Server::Server(Config* config)
    : enet_wrapper::ENetServer{}
    , m_config{ config }
    , m_peer{ nullptr }
{
    m_http = new Http{ config };
    m_client = new client::Client{ m_config, m_http, this };
}

Server::~Server()
{
    delete m_http;
    delete m_peer.m_gt_client;
    delete m_client;
}

bool Server::start()
{
    if (!m_http->listen("0.0.0.0", 443)) {
        return false;
    }

    if (!create_host(m_config->get_host().m_port, 1, 1)) {
        spdlog::error("Failed to create ENet server host.");
        return false;
    }

    start_service();
    spdlog::info("ENet server listening on port {}.", m_config->get_host().m_port);
    return true;
}

void Server::on_connect(ENetPeer* peer)
{
    spdlog::info("New client connected to proxy server.");

    m_peer.m_gt_client = new player::Peer{ peer };
    m_client->set_gt_client_peer(m_peer.m_gt_client);
    m_client->start();
}

void Server::on_receive(ENetPeer* peer, ENetPacket* packet)
{
    if (!m_peer.m_gt_server) {
        return;
    }

    if (!m_peer.m_gt_server->is_connected()) {
        return;
    }

    if (!process_packet(peer, packet)) {
        return;
    }

    m_peer.m_gt_server->send_packet_packet(packet);
}

void Server::on_disconnect(ENetPeer* peer)
{
    spdlog::info("Client disconnected from proxy server.");

    if (m_peer.m_gt_server && m_peer.m_gt_server->is_connected()) {
        m_peer.m_gt_server->disconnect();
    }

    delete m_peer.m_gt_client;
    m_peer.m_gt_client = nullptr;
    m_client->set_gt_client_peer(nullptr);
}

bool Server::process_packet(ENetPeer* peer, ENetPacket* packet)
{
    player::eNetMessageType message_type{ player::message_type_to_string(packet) };
    std::string message_data{ player::get_text(packet) };

    if (message_type != player::NET_MESSAGE_GAME_PACKET) {
        utils::TextParse text_parse{ message_data };
        if (!text_parse.empty()) {
            spdlog::info(
                "Outgoing MessagePacket:\n{} [{}]:\n{}\n",
                magic_enum::enum_name(message_type),
                message_type,
                fmt::join(text_parse.get_all_array(), "\r\n")
            );
        }
    }

    switch (message_type) {
        case player::NET_MESSAGE_GENERIC_TEXT: {
            if (message_data.find("action|input") != std::string::npos) {
                utils::TextParse text_parse{ message_data };
                if (text_parse.get("text", 1).empty()) {
                    break;
                }

                std::vector<std::string> token{ utils::TextParse::string_tokenize( message_data.substr(
                    message_data.find("text|") + 5,
                    message_data.length() - message_data.find("text|") - 1
                ), " " )};

                if (token[0] == m_config->get_command().m_prefix + "warp") {
                    std::string world{token[1]};
                    m_peer.m_gt_server->send_packet(
                        player::eNetMessageType::NET_MESSAGE_GAME_MESSAGE,
                        fmt::format("action|join_request\nname|{}\ninvitedWorld|0", world)
                    );
                    return false;
                }
            }
            else if (message_data.find("requestedName") != std::string::npos) {
                auto md5{ 
                    [](std::string_view input) -> std::string {
                        std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
                        std::uint32_t digest_len{};

                        EVP_MD_CTX* ctx{ EVP_MD_CTX_new() };
                        EVP_DigestInit_ex(ctx, EVP_md5(), nullptr);
                        EVP_DigestUpdate(ctx, input.data(), input.length());
                        EVP_DigestFinal_ex(ctx, digest.data(), &digest_len);
                        EVP_MD_CTX_free(ctx);

                        std::string md5_string{};
                        md5_string.reserve(32);

                        for (int i{ 0 }; i < 16; i++) {
                            md5_string += fmt::format("{:02X}", digest[i]);
                        }

                        return md5_string;
                    } 
                };

                auto generate_klv{ 
                    [&](
                        std::string_view game_version, 
                        std::int32_t device_id_hash, 
                        std::string_view rid, 
                        std::uint16_t protocol
                    ) -> std::string {
                        constexpr std::array salts = {
                            "42e2ae20305244ddaf9b0de5e897fc74",
                            "ccc18d2e2ca84e0a81ba29a0af2edc9c",
                            "92e9bf1aad214c69b1f3a18a03aae8dc",
                            "58b92130c89c496b96164b776d956242"
                        };

                        return md5(fmt::format(
                            "{}{}{}{}{}{}{}{}",
                            game_version,
                            salts[0],
                            protocol,
                            salts[1],
                            device_id_hash,
                            salts[2],
                            rid,
                            salts[3]
                        ));
                    } 
                };

                static randutils::pcg_rng gen{ utils::random::get_generator_local() };
                static std::string mac{ utils::random::generate_mac(gen) };
                static std::int32_t mac_hash{ utils::proton_hash(fmt::format("{}RT", mac).c_str()) };
                static std::string rid{ utils::random::generate_hex(gen, 32, true) };
                static std::string wk{ utils::random::generate_hex(gen, 32, true) };
                static std::string device_id{ utils::random::generate_hex(gen, 16, true) };
                static std::int32_t device_id_hash{ utils::proton_hash(fmt::format("{}RT", device_id).c_str()) };

                utils::TextParse text_parse{ message_data };

                text_parse.add_key_once("klv|");
                text_parse.set("game_version", m_config->get_server().m_game_version);

                // text_parse.set("protocol", m_config->m_server.protocol);
                // text_parse.set("platformID", m_config->m_server.platformID);
                text_parse.set("mac", mac);
                text_parse.set("rid", rid);
                text_parse.set("wk", wk);
                text_parse.set("hash", device_id_hash);
                text_parse.set("hash2", mac_hash);
                text_parse.set(
                    "klv", 
                    generate_klv(
                        text_parse.get("game_version", 1),
                        text_parse.get<std::int32_t>("hash", 1),
                        text_parse.get("rid", 1),
                        text_parse.get<std::uint16_t>("protocol", 1)
                    )
                );

                spdlog::debug("{}", text_parse.get_all_raw());
                m_peer.m_gt_server->send_packet(message_type, text_parse.get_all_raw());
                return false;
            }

            break;
        }
        case player::NET_MESSAGE_GAME_MESSAGE: {
            if (message_data.find("action|quit") != std::string::npos && message_data.length() <= 15) {
                m_peer.m_gt_client->disconnect();
            }

            break;
        }
        case player::NET_MESSAGE_GAME_PACKET: {
            player::GameUpdatePacket* game_update_packet{ player::get_struct(packet) };
            return process_tank_update_packet(peer, game_update_packet);
        }
        default:
            break;
    }

    return true;
}

bool Server::process_tank_update_packet(ENetPeer* peer, player::GameUpdatePacket* game_update_packet) const
{
    if (game_update_packet->type != player::PACKET_CALL_FUNCTION) {
        std::uint8_t* extended_data{ player::get_extended_data(game_update_packet) };
        std::vector<std::uint8_t> extended_data_vector{ extended_data, extended_data + game_update_packet->data_size };

        spdlog::info(
            "Outgoing TankUpdatePacket:\n [{}]{}{}",
            game_update_packet->type,
            magic_enum::enum_name(static_cast<player::ePacketType>(game_update_packet->type)),
            extended_data
            ? fmt::format("\n > extended_data: {}", spdlog::to_hex(extended_data_vector))
            : ""
        );
    }

    switch (game_update_packet->type) {
        case player::PACKET_DISCONNECT:
            m_peer.m_gt_client->disconnect_now();
            break;
        default:
            break;
    }

    return true;
}
}
