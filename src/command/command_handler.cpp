#include <fmt/chrono.h>
#include <fmt/ranges.h>

#include "command_handler.h"
#include "../config.h"
#include "../server/server.h"
#include "../utils/textparse.h"

namespace command {
    CommandHandler::CommandHandler(server::Server *server)
        : m_server(server)
    {
        m_commands.push_back(
            new Command({ "help", { "?" }, "Displays this help message" }, [this](const std::vector<std::string>& args) {
                if (!args.empty()) {
                    auto it = std::find_if(m_commands.begin(), m_commands.end(), [args](const Command* command) {
                        if (command->get_name() != args[0]) {
                            std::vector<std::string> aliases = command->get_aliases();
                            return std::find(aliases.cbegin(), aliases.cend(), args[0]) != aliases.cend();
                        }

                        return command->get_name() == args[0];
                    });

                    if (it != m_commands.end())
                        m_server->get_player()->send_log((*it)->get_description());
                    else
                        m_server->get_player()->send_log("`4Unknown command. ``Enter `$!help`` for a list of valid commands.");
                    return;
                }

                std::string commands{ ">> Commands: " };
                for (auto &command : m_commands) {
                    commands.append(Config::get().config()["command"]["prefix"]);
                    commands.append(command->get_name());
                    commands.push_back(' ');
                }

                commands.pop_back();
                m_server->get_player()->send_log(commands);
            })
        );
        m_commands.push_back(
            new Command({ "warp", {}, "Warps you to a world" }, [this](const std::vector<std::string> &args) {
                if (args.empty()) {
                    m_server->get_player()->send_log("`4Usage: ``!warp <world name>");
                    return;
                }

                if (args[0] == "exit") {
                    m_server->get_player()->send_log("`4You cannot warp to the exit world.");
                    return;
                }

                if (args[0].size() > 23) {
                    m_server->get_player()->send_log("`4World name too long, try again.");
                    return;
                }

                m_server->get_client_player()->send_packet(player::NET_MESSAGE_GAME_MESSAGE, "action|quit_to_exit");
                m_server->get_player()->send_log(fmt::format("Warping to {}...", args[0]));
                m_server->get_client_player()->send_packet(
                    player::NET_MESSAGE_GAME_MESSAGE,
                    fmt::format(
                        "action|join_request\n"
                        "name|{}\n"
                        "invitedWorld|0", args[0]));
            })
        );
    }

    CommandHandler::~CommandHandler() {
        for (auto &command : m_commands)
            delete command;
        m_commands.clear();
    }

    bool CommandHandler::handle(const std::string &string) {
        std::vector<std::string> args = utils::TextParse::string_tokenize(string, " ");
        if (args.empty())
            return false;

        if (!args[0].starts_with(Config::get().config()["command"]["prefix"].get<std::string>()))
            return false;

        std::string command_name = args[0].substr(1);
        std::transform(command_name.cbegin(), command_name.cend(), command_name.begin(), ::tolower);
        args.erase(args.cbegin());

        for (auto &command : m_commands) {
            if (command->get_name() != command_name) {
                std::vector<std::string> aliases = command->get_aliases();
                auto it = std::find(aliases.cbegin(), aliases.cend(), command_name);
                if (it == aliases.cend())
                    continue;
            }

            m_server->get_player()->send_log(fmt::format("`6{}``", string));
            command->call(args);
            return true;
        }
        return false;
    }
}
