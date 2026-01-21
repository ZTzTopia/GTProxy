#include "command_registry.hpp"

#include <iterator>
#include <sstream>
#include <spdlog/spdlog.h>

namespace command {
void CommandRegistry::add(std::unique_ptr<ICommand> cmd)
{
    std::string key{ cmd->name() };
    commands_[std::move(key)] = std::move(cmd);
}

ICommand* CommandRegistry::get(std::string_view name) const
{
    if (const auto it = commands_.find(std::string(name)); it != commands_.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<std::pair<std::string, std::string>> CommandRegistry::get_all_commands() const
{
    std::vector<std::pair<std::string, std::string>> result;
    result.reserve(commands_.size());
    for (const auto& [name, cmd] : commands_) {
        result.emplace_back(name, cmd->description());
    }
    return result;
}

bool CommandRegistry::is_command(std::string_view input) const
{
    return !input.empty() && input[0] == prefix_;
}

bool CommandRegistry::execute(
    std::string_view input,
    network::Server& server,
    network::Client& client,
    event::Dispatcher& dispatcher,
    core::Scheduler& scheduler)
{
    if (!is_command(input)) {
        return false;
    }

    auto [name, args] = parse(input);
    if (name.empty()) {
        return false;
    }

    auto* cmd = get(name);
    if (!cmd) {
        return false;
    }

    const Context ctx{
        std::move(args),
        std::string(input),
        server,
        client,
        dispatcher,
        scheduler,
        *this
    };

    const auto result = cmd->execute(ctx);
    if (result != Result::Success) {
        spdlog::warn("Command '{}' returned {}", name,
            result == Result::InvalidArguments ? "InvalidArguments" : "Failed");
    }
    return true;
}

std::pair<std::string, std::vector<std::string>> CommandRegistry::parse(std::string_view input) const
{
    if (input.empty() || input[0] != prefix_) {
        return {};
    }

    std::string clean{ input.substr(1) };
    if (clean.empty()) {
        return {};
    }

    std::istringstream iss{ clean };
    std::vector<std::string> tokens{
        std::istream_iterator<std::string>{ iss },
        std::istream_iterator<std::string>{}
    };

    if (tokens.empty()) {
        return {};
    }

    std::string name = std::move(tokens[0]);
    tokens.erase(tokens.begin());

    return { std::move(name), std::move(tokens) };
}
}
