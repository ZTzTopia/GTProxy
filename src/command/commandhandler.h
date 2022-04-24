#pragma once
#include <string>
#include <vector>

#include "command.h"

namespace server {
    class Server;
}

namespace command {
    class CommandHandler {
    public:
        CommandHandler(server::Server *server);
        ~CommandHandler();

        bool handle(const std::string &string);

    private:
        server::Server *m_server;
        std::vector<Command *> m_commands;
    };
}