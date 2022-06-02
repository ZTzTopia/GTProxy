#include "local_player.h"
#include "../client/client.h"
#include "../server/server.h"

namespace player {
    LocalPlayer::LocalPlayer(player::Player* player)
        : m_player(player), m_net_id(0), m_flags(NONE), m_user_id(0), m_pos(), m_goal_pos(-1, -1), m_next_move_pos()
        , m_auto_collect_radius(1)
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
        // Auto collect dropped items/objects.
        if (has_flags(AUTO_COLLECT)) {
            for (auto& object : m_world->object_map.objects) {
                utils::math::Vec2<int> object_pos{ static_cast<int>(object.pos.x), static_cast<int>(object.pos.y) };
                if (utils::math::distance(object_pos, m_pos) <= 5 * 32) {
                    if (utils::math::distance(object_pos, m_pos) <= m_auto_collect_radius * 32) {
                        player::GameUpdatePacket game_update_packet;
                        game_update_packet.type = PACKET_ITEM_ACTIVATE_OBJECT_REQUEST;
                        game_update_packet.pos_x = object.pos.x;
                        game_update_packet.pos_y = object.pos.y;
                        game_update_packet.object_id = static_cast<int32_t>(object.drop_id_offset);
                        client->get_player()->send_raw_packet(player::NET_MESSAGE_GAME_PACKET, &game_update_packet);
                    }
                }
                else {
                    // If the object radius is greater than 5, we need to use pathfinding to take the object.
                    if (utils::math::distance(object_pos, m_pos) <= m_auto_collect_radius * 32) {
                        m_goal_pos = object_pos;
                        break;
                    }
                }
            }
        }

        // Pathfinding.
        if (m_goal_pos.x == -1 && m_goal_pos.y == -1)
            return;

        // TODO: Make this more efficient.
        if (std::abs(m_goal_pos.x - m_pos.x) <= 32 && std::abs(m_goal_pos.y - m_pos.y) <= 32) {
            int goal_pos_y = (m_goal_pos.y / 32) * 32;
            client->get_server()->get_player()->send_variant(
                { "OnSetPos", { static_cast<float>(m_goal_pos.x), static_cast<float>(goal_pos_y) } }, m_net_id);

            m_goal_pos = { -1, -1 };
            return;
        }

        int x = m_pos.x / 32;
        int y = m_pos.y / 32;

        int goal_x = m_goal_pos.x / 32;
        int goal_y = m_goal_pos.y / 32;

        std::vector<utils::math::Vec2<int32_t>> path{ m_world->find_path({ x, y }, { goal_x, goal_y }, items) };
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

        client->get_server()->get_player()->send_variant(
            { "OnSetPos", { static_cast<float>(next_pos.x * 32), static_cast<float>(next_pos.y * 32) } }, m_net_id);

        m_pos = { next_pos.x * 32, next_pos.y * 32 };
    }
}
