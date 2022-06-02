#include "command_manager.h"
#include "../config.h"
#include "../server/server.h"
#include "../utils/text_parse.h"

namespace command {
    CommandManager::CommandManager()
    {
        Command cmd_help{};
        cmd_help.set_name("help");
        cmd_help.set_aliases({ "h", "?" });
        cmd_help.set_description("Prints this help message.");
        cmd_help.set_function([this](auto&& PH1) {
            command_help(std::forward<decltype(PH1)>(PH1));
        });
        m_commands.insert({ cmd_help.get_name(), new Command{ cmd_help } });

        Command cmd_warp{};
        cmd_warp.set_name("warp");
        cmd_warp.set_description("Warps you to a world.");
        cmd_warp.set_function([this](auto&& PH1) {
            command_warp(std::forward<decltype(PH1)>(PH1));
        });
        m_commands.insert({ cmd_warp.get_name(), new Command{ cmd_warp } });

        Command cmd_random_warp{};
        cmd_random_warp.set_name("randomwarp");
        cmd_random_warp.set_aliases({ "rw", "rwarp", "randomw" });
        cmd_random_warp.set_description("Warps you to a random world.");
        cmd_random_warp.set_function([this](auto&& PH1) {
            command_random_warp(std::forward<decltype(PH1)>(PH1));
        });
        m_commands.insert({ cmd_random_warp.get_name(), new Command{ cmd_random_warp } });

        Command cmd_nick{};
        cmd_nick.set_name("nickname");
        cmd_nick.set_aliases({ "nick" });
        cmd_nick.set_description("Change your player nickname.");
        cmd_nick.set_function([this](auto&& PH1) {
            command_nick(std::forward<decltype(PH1)>(PH1));
        });
        m_commands.insert({ cmd_nick.get_name(), new Command{ cmd_nick } });

        Command cmd_skin{};
        cmd_skin.set_name("skin");
        cmd_skin.set_description("Change your player skin.");
        cmd_skin.set_function([this](auto&& PH1) {
            command_skin(std::forward<decltype(PH1)>(PH1));
        });
        m_commands.insert({ cmd_skin.get_name(), new Command{ cmd_skin } });

        Command cmd_fast_trash{};
        cmd_fast_trash.set_name("fasttrash");
        cmd_fast_trash.set_aliases({ "ft" });
        cmd_fast_trash.set_description("Fast trash item.");
        cmd_fast_trash.set_function([this](auto&& PH1) {
            command_fast_trash(std::forward<decltype(PH1)>(PH1));
        });
        m_commands.insert({ cmd_fast_trash.get_name(), new Command{ cmd_fast_trash } });

        Command cmd_fast_drop{};
        cmd_fast_drop.set_name("fastdrop");
        cmd_fast_drop.set_aliases({ "fd" });
        cmd_fast_drop.set_description("Fast drop item.");
        cmd_fast_drop.set_function([this](auto&& PH1) {
            command_fast_drop(std::forward<decltype(PH1)>(PH1));
        });
        m_commands.insert({ cmd_fast_drop.get_name(), new Command{ cmd_fast_drop } });

        Command cmd_fast_wrench{};
        cmd_fast_wrench.set_name("fastwrench");
        cmd_fast_wrench.set_aliases({ "fw" });
        cmd_fast_wrench.set_description("Fast pull, kick, and world ban player.");
        cmd_fast_wrench.set_function([this](auto&& PH1) {
            command_fast_wrench(std::forward<decltype(PH1)>(PH1));
        });
        m_commands.insert({ cmd_fast_wrench.get_name(), new Command{ cmd_fast_wrench } });

        Command cmd_pull_all{};
        cmd_pull_all.set_name("pullall");
        cmd_pull_all.set_aliases({ "pall", "pulla", "pa" });
        cmd_pull_all.set_description("Pull all the players in the world.");
        cmd_pull_all.set_function([this](auto&& PH1) {
            command_pull_all(std::forward<decltype(PH1)>(PH1));
        });
        m_commands.insert({ cmd_pull_all.get_name(), new Command{ cmd_pull_all } });

        Command cmd_kick_all{};
        cmd_kick_all.set_name("kickall");
        cmd_kick_all.set_aliases({ "kall", "kicka", "ka" });
        cmd_kick_all.set_description("Kick all the players in the world.");
        cmd_kick_all.set_function([this](auto&& PH1) {
            command_kick_all(std::forward<decltype(PH1)>(PH1));
        });
        m_commands.insert({ cmd_kick_all.get_name(), new Command{ cmd_kick_all } });

        Command cmd_world_ban_all{};
        cmd_world_ban_all.set_name("worldbanall");
        cmd_world_ban_all.set_aliases({ "wba", "wball", "worldbana", "bana", "ba" });
        cmd_world_ban_all.set_description("World ban all the players in the world.");
        cmd_world_ban_all.set_function([this](auto&& PH1) {
            command_world_ban_all(std::forward<decltype(PH1)>(PH1));
        });
        m_commands.insert({ cmd_world_ban_all.get_name(), new Command{ cmd_world_ban_all } });

        Command cmd_message_all{};
        cmd_message_all.set_name("messageall");
        cmd_message_all.set_aliases({ "msgall", "mall", "messagea", "msga" "ma" });
        cmd_message_all.set_description("Message all the players in the world.");
        cmd_message_all.set_function([this](auto&& PH1) {
            command_message_all(std::forward<decltype(PH1)>(PH1));
        });
        m_commands.insert({ cmd_message_all.get_name(), new Command{ cmd_message_all } });

        Command cmd_trade_all{};
        cmd_trade_all.set_name("tradeall");
        cmd_trade_all.set_aliases({ "trdall", "tall", "tradea", "trda" "ta" });
        cmd_trade_all.set_description("Pull all the players in the world.");
        cmd_trade_all.set_function([this](auto&& PH1) {
            command_trade_all(std::forward<decltype(PH1)>(PH1));
        });
        m_commands.insert({ cmd_trade_all.get_name(), new Command{ cmd_trade_all } });
    }

    CommandManager::~CommandManager()
    {
        for (auto& command : m_commands) {
            delete command.second;
        }

        m_commands.clear();
    }

    bool CommandManager::try_find_and_fire_command(server::Server* server, const std::string& text)
    {
        std::vector<std::string> args = utils::TextParse::string_tokenize(text, " ");
        if (args.empty()) return false;

        std::string prefix{ Config::get().config()["command"]["prefix"].get<std::string>() };
        if (!args[0].starts_with(prefix))
            return false;

        std::string command_name = args[0].substr(prefix.length());
        std::transform(command_name.cbegin(), command_name.cend(), command_name.begin(), ::tolower);
        args.erase(args.cbegin());

        return std::ranges::any_of(m_commands, [&](const auto& command) {
            std::vector<std::string> aliases = command.second->get_aliases();
            if (command.second->get_name() == command_name || std::ranges::any_of(aliases, [&](const auto& alias) {
                return alias == command_name;
            })) {
                server->get_player()->send_log(fmt::format("`6{}``", text));

                CommandContext ctx{};
                ctx.prefix = prefix;
                ctx.server_peer = server->get_player();
                ctx.client_peer = server->get_client_player();
                ctx.local_player = server->get_client()->get_local_player();
                ctx.args = args;

                command.second->get_function()(ctx);
                return true;
            }

            return false;
        });
    }
}
