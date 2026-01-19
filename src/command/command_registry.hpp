#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "command.hpp"

namespace command {
class CommandRegistry {
public:
    explicit CommandRegistry(const char prefix = '/')
        : prefix_{ prefix }
    { }

    void set_prefix(char prefix) { prefix_ = prefix; }

    void add(std::unique_ptr<ICommand> cmd);

    template <typename Func>
    void add(const std::string& name, Func&& func)
    {
        add(make_command(name, std::forward<Func>(func)));
    }

    [[nodiscard]] ICommand* get(std::string_view name) const;

    [[nodiscard]] bool is_command(std::string_view input) const;

    [[nodiscard]] bool execute(
        std::string_view input,
        network::Server& server,
        network::Client& client,
        event::Dispatcher& dispatcher,
        std::shared_ptr<core::Scheduler> scheduler
    );

    [[nodiscard]] char prefix() const { return prefix_; }

private:
    [[nodiscard]] std::pair<std::string, std::vector<std::string>> parse(std::string_view input) const;

private:
    char prefix_;
    std::unordered_map<std::string, std::unique_ptr<ICommand>> commands_;
};
}
