#include "local_player.h"

namespace player {
    LocalPlayer::LocalPlayer() : m_net_id{ 0 }, m_flags{ NONE }
    {
        m_items = new PlayerItems{};
        m_world = new World{};
    }

    LocalPlayer::~LocalPlayer()
    {
        delete m_items;
        delete m_world;
    }
}
