#pragma once
#include "command.h"

namespace server {
    class Server;
}

namespace command {
    class CommandManager {
    public:
        CommandManager();
        ~CommandManager();

        bool try_find_and_fire_command(server::Server* server, const std::string& text);

    private:
        void command_help(const CommandContext& ctx);
        void command_warp(const CommandContext& ctx);
        void command_random_warp(const CommandContext& ctx);
        void command_nick(const CommandContext& ctx);
        void command_skin(const CommandContext& ctx);
        void command_fast_trash(const CommandContext& ctx);
        void command_fast_drop(const CommandContext& ctx);
        void command_fast_wrench(const CommandContext& ctx);
        void command_pull_all(const CommandContext& ctx);
        void command_kick_all(const CommandContext& ctx);
        void command_world_ban_all(const CommandContext& ctx);
        void command_message_all(const CommandContext& ctx);
        void command_trade_all(const CommandContext& ctx);
        void command_auto_collect(const CommandContext& ctx);

    private:
        std::unordered_map<std::string, Command*> m_commands;
    };
}