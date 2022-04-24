#include "command.h"

namespace command {
    Command::Command(const std::string &name, const std::string &description, const std::function<void(const std::vector<std::string> &)> &callback)
        : name(name)
        , description(description)
        , callback(callback)
    {}

    void Command::call(const std::vector<std::string> &args) {
        callback(args);
    }
}
