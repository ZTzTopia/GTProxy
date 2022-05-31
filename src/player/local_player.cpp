#include "local_player.h"
#include "../client/client.h"
#include "../server/server.h"

namespace player {
    LocalPlayer::LocalPlayer(player::Player* player)
        : m_player(player), m_net_id(0), m_flags(NONE), m_user_id(0), m_pos(), m_goal_pos(-1, -1), m_next_move_pos()
    {
        m_items = new PlayerItems{};
        m_world = new World{};
    }

    LocalPlayer::~LocalPlayer()
    {
        delete m_items;
        delete m_world;
    }

    void LocalPlayer::on_update(client::Client* client, items::Items* items)
    {
        if (m_goal_pos.x == -1 && m_goal_pos.y == -1) {
            return;
        }

        int x = m_pos.x / 32;
        int y = m_pos.y / 32;

        spdlog::info("LocalPlayer::on_update: x={}, y={}", x, y);

        std::vector<utils::math::Vec2<int32_t>> path = m_world->find_path({ x, y }, m_goal_pos, items);
        if (path.empty()) {
            m_goal_pos = { -1, -1 };
            return;
        }

        utils::math::Vec2<int32_t> next_pos{ path.back() };
        if (std::abs((next_pos.x * 32) - m_pos.x) <= 8 && std::abs((next_pos.y * 32) - m_pos.y) <= 32) {
            path.erase(path.begin());
            if (path.empty()) {
                m_goal_pos = { -1, -1 };
                return;
            }

            next_pos = path.back();
        }

        spdlog::info("next_pos: {}, {}, x, y: {}, {}", next_pos.x, next_pos.y, x, y);

        client->get_server()->get_player()->send_variant(
            { "OnSetPos", { static_cast<float>(next_pos.x * 32), static_cast<float>(next_pos.y * 32) } }, m_net_id);

        m_pos = { next_pos.x * 32, next_pos.y * 32 };

        /*if (next_pos.x < x) {
            player::GameUpdatePacket game_update_packet{};
            game_update_packet.pos_x = static_cast<float>(m_pos.x - 32);
            game_update_packet.pos_y = static_cast<float>(m_pos.y);
            client->get_server()->get_player()->send_variant(
                { "OnSetPos", { game_update_packet.pos_x, game_update_packet.pos_y } }, m_net_id);
        }
        else if (next_pos.x > x) {
            player::GameUpdatePacket game_update_packet{};
            game_update_packet.pos_x = static_cast<float>(m_pos.x + 32);
            game_update_packet.pos_y = static_cast<float>(m_pos.y);
            client->get_server()->get_player()->send_variant(
                { "OnSetPos", { game_update_packet.pos_x, game_update_packet.pos_y } }, m_net_id);
        }
        else if (next_pos.y < y) {
            player::GameUpdatePacket game_update_packet{};
            game_update_packet.pos_x = static_cast<float>(m_pos.x);
            game_update_packet.pos_y = static_cast<float>(m_pos.y - 32);
            client->get_server()->get_player()->send_variant(
                { "OnSetPos", { game_update_packet.pos_x, game_update_packet.pos_y } }, m_net_id);
        }*/
    }
}
