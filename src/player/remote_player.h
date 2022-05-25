#pragma once
#include <cstdint>
#include <string>

namespace player {
    class RemotePlayer {
    public:
        RemotePlayer();
        ~RemotePlayer() = default;

        void set_net_id(uint32_t net_id) { m_net_id = net_id; }
        [[nodiscard]] uint32_t get_net_id() const { return m_net_id; }

        void set_raw_name(const std::string& raw_name) { m_raw_name = raw_name; }
        [[nodiscard]] const std::string& get_raw_name() const { return m_raw_name; }

        void set_display_name(const std::string& display_name) { m_display_name = display_name; }
        [[nodiscard]] const std::string& get_display_name() const { return m_display_name; }

    private:
        uint32_t m_net_id;
        std::string m_raw_name;
        std::string m_display_name;
    };
}
