#pragma once
#include <cstdint>

#include "player_items.h"
#include "../world/world.h"

namespace player {
    enum eFlag : uint8_t {
        NONE = 0,
        FAST_DROP = 1 << 0,
        FAST_TRASH = 1 << 1,
        FAST_WRENCH_PULL = 1 << 2,
        FAST_WRENCH_KICK = 1 << 3,
        FAST_WRENCH_BAN = 1 << 4,
    };

    inline eFlag operator|(eFlag a, eFlag b)
    {
        return static_cast<eFlag>(static_cast<int>(a) | static_cast<int>(b));
    }

    inline eFlag operator&(eFlag a, eFlag b)
    {
        return static_cast<eFlag>(static_cast<int>(a) & static_cast<int>(b));
    }

    inline eFlag operator~(eFlag a)
    {
        return static_cast<eFlag>(~static_cast<int>(a));
    }

    inline eFlag& operator^(eFlag& a, eFlag b)
    {
        return a = static_cast<eFlag>(static_cast<int>(a) ^ static_cast<int>(b));
    }

    inline eFlag& operator|=(eFlag& a, eFlag b)
    {
        a = a | b;
        return a;
    }

    inline eFlag& operator&=(eFlag& a, eFlag b)
    {
        a = a & b;
        return a;
    }

    inline eFlag& operator^=(eFlag& a, eFlag b)
    {
        a = a ^ b;
        return a;
    }

    class LocalPlayer {
    public:
        LocalPlayer();
        ~LocalPlayer();

        void on_update();

        void set_net_id(uint32_t net_id) { m_net_id = net_id; }
        [[nodiscard]] uint32_t get_net_id() const { return m_net_id; }

        void set_flags(eFlag flags) { m_flags |= flags; }
        void unset_flags(eFlag flags) { m_flags &= ~flags; }
        void toggle_flags(eFlag flags) { m_flags ^= flags; }
        [[nodiscard]] eFlag get_flags() const { return m_flags; }
        [[nodiscard]] bool has_flags(eFlag flags) const { return (m_flags & flags) == flags; }

        void set_user_id(uint32_t user_id) { m_user_id = user_id; }
        [[nodiscard]] uint32_t get_user_id() const { return m_user_id; }

        void set_items(PlayerItems* items) { m_items = items; }
        [[nodiscard]] PlayerItems* get_items() const { return m_items; }

        void set_world(World* world) { m_world = world; }
        [[nodiscard]] World* get_world() const { return m_world; }

        void set_pos(const utils::math::Vec2<int>& pos) { m_pos = pos; }
        [[nodiscard]] const utils::math::Vec2<int>& get_pos() const { return m_pos; }

    private:
        uint32_t m_net_id;
        eFlag m_flags;
        uint32_t m_user_id;

        PlayerItems* m_items;
        World* m_world;

        utils::math::Vec2<int> m_pos;
    };
}
