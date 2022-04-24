#include "command.h"

#include <utility>

namespace command {
    Command::Command(std::string name, std::string description, std::function<void(const std::vector<std::string> &)> callback)
        : m_name(std::move(name))
        , m_description(std::move(description))
        , m_callback(std::move(callback))
    {}

    void Command::call(const std::vector<std::string> &args) {
        m_callback(args);
    }
}
