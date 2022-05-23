#pragma once
#include <functional>
#include <string>
#include <utility>

#include "../player/player.h"
#include "../player/local_player.h"
#include "../player/remote_player.h"

namespace command {
    struct CommandContext {
        std::string name;
        std::vector<std::string> aliases;
        std::string description;
    };

    struct CommandCallContext {
        std::string prefix;
        player::Player* local_peer;
        player::Player* server_peer;
        player::LocalPlayer* local_player;
        std::unordered_map<uint32_t, player::RemotePlayer*> remote_player;
    };

    class Command {
    public:
        Command(CommandContext context,
            std::function<void(const CommandCallContext&, const std::vector<std::string>&)> callback)
            : m_context(std::move(context)), m_callback(std::move(callback)) {}
        ~Command() = default;

        void call(const CommandCallContext& command_call_context, const std::vector<std::string>& args)
        {
            m_callback(command_call_context, args);
        }

        [[nodiscard]] CommandContext get_context() const { return m_context; }
        [[nodiscard]] std::string get_name() const { return m_context.name; }
        [[nodiscard]] std::vector<std::string> get_aliases() const { return m_context.aliases; }
        [[nodiscard]] std::string get_description() const { return m_context.description; }

    private:
        CommandContext m_context;
        std::function<void(const CommandCallContext&, const std::vector<std::string>&)> m_callback;
    };
}// namespace command