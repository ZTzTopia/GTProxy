#include "commandhandler.h"
#include "../server/server.h"
#include "../utils/textparse.h"

namespace command {
    CommandHandler::CommandHandler(server::Server *server)
        : m_server(server)
    {
        m_commands.push_back(
            new Command("help", "Displays this help message", [this](const std::vector<std::string>& args) {
                if (!args.empty()) {
                    auto it = std::find_if(m_commands.begin(), m_commands.end(), [&args](const Command* command) {
                        return command->get_name() == args[0];
                    });

                    if (it != m_commands.end()) {
                        m_server->get_player()->send_log((*it)->get_description());
                    }
                    else {
                        m_server->get_player()->send_log("`4Unknown command. ``Enter `$!help`` for a list of valid commands.");
                    }

                    return;
                }

                std::string commands;

                commands.append(">> Commands: ");
                for (auto &command : m_commands) {
                    commands += '!';
                    commands += command->get_name();
                    commands += ' ';
                }

                m_server->get_player()->send_log(commands);
            })
        );
    }

    CommandHandler::~CommandHandler() {
        for (auto &command : m_commands) {
            delete command;
        }

        m_commands.clear();
    }

    bool CommandHandler::handle(const std::string &string) {
        std::vector<std::string> args = utils::TextParse::string_tokenize(string, " ");
        if (args.empty()) {
            return false;
        }

        if (!args[0].starts_with("!")) {
            return false;
        }

        std::string command_name = args[0].substr(1);
        std::transform(command_name.begin(), command_name.end(), command_name.begin(), ::tolower);

        args.erase(args.begin());

        for (auto &command : m_commands) {
            if (command->get_name() == command_name) {
                command->call(args);
                return true;
            }
        }

        return false;
    }
}