#pragma once
#include <cstdint>

namespace player {
    class RemotePlayer {
    public:
        RemotePlayer();
        ~RemotePlayer() = default;

        void set_net_id(uint32_t net_id) { m_net_id = net_id; }
        [[nodiscard]] uint32_t get_net_id() const { return m_net_id; }

    private:
        uint32_t m_net_id;
    };
}
