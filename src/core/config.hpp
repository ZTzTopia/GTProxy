#pragma once
#include <string>

namespace core {
class Config {
    struct Host {
        std::uint16_t m_port;
    };

    struct Server {
        std::string m_host;
        std::string m_game_version;
        std::string m_platform_id;
        std::uint8_t m_protocol;
    };

    struct Command {
        std::string m_prefix;
    };

    struct Misc {
        bool m_force_update_game_version;
        bool m_force_update_protocol;
    };

public:
    Config();
    ~Config() = default;

    bool create(const std::string& file);
    bool load(const std::string& file);

public:
    [[nodiscard]] const Host& get_host() const { return m_host; }
    [[nodiscard]] const Server& get_server() const { return m_server; }
    [[nodiscard]] const Command& get_command() const { return m_command; }
    [[nodiscard]] const Misc& get_misc() const { return m_misc; }

private:
    Host m_host;
    Server m_server;
    Command m_command;
    Misc m_misc;
};
}
