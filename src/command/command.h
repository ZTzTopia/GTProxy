#pragma once
#include <functional>
#include <string>
#include <utility>

#include "../player/player.h"
#include "../player/local_player.h"
#include "../player/remote_player.h"

namespace command {
    struct CommandContext {
        std::string prefix;
        player::Player* server_peer;
        player::Player* client_peer;
        player::LocalPlayer* local_player;
        std::vector<std::pair<std::string, player::RemotePlayer*>> remote_players;
        std::vector<std::string> args;
    };

    class Command {
    public:
        Command() = default;
        ~Command()
        {
            m_aliases.clear();
        }

        [[nodiscard]] std::string get_name() const { return m_name; }
        void set_name(const std::string& name) { m_name = name; }

        [[nodiscard]] std::vector<std::string> get_aliases() const { return m_aliases; }
        void set_aliases(const std::vector<std::string>& aliases) { m_aliases = aliases; }

        [[nodiscard]] std::string get_description() const { return m_description; }
        void set_description(const std::string& description) { m_description = description; }

        [[nodiscard]] std::function<void(const CommandContext&)> get_function() const { return m_function; }
        void set_function(const std::function<void(const CommandContext&)>& function) { m_function = function; }

    private:
        std::string m_name;
        std::vector<std::string> m_aliases;
        std::string m_description;

        std::function<void(const CommandContext&)> m_function;
    };
}// namespace command