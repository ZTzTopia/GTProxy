#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "../core/scheduler.hpp"
#include "../event/event.hpp"
#include "../network/client.hpp"
#include "../network/server.hpp"

namespace command {
class CommandRegistry;

enum class Result {
    Success,
    InvalidArguments,
    Failed
};

struct Context {
    std::vector<std::string> args;
    std::string raw_input;
    network::Server& server;
    network::Client& client;
    event::Dispatcher& dispatcher;
    std::shared_ptr<core::Scheduler> scheduler;
    const CommandRegistry& registry;
};

class ICommand {
public:
    virtual ~ICommand() = default;

    [[nodiscard]] virtual std::string_view name() const = 0;

    virtual std::string description() const = 0;

    virtual Result execute(const Context& ctx) = 0;
};

template <typename Func>
class LambdaCommand final : public ICommand {
public:
    LambdaCommand(std::string name, std::string description, Func func)
        : name_{ std::move(name) }
        , description_{ std::move(description) }
        , func_{ std::move(func) }
    { }

    [[nodiscard]] std::string_view name() const override { return name_; }
    [[nodiscard]] std::string description() const override { return description_; }

    Result execute(const Context& ctx) override
    {
        if constexpr (std::is_same_v<std::invoke_result_t<Func, const Context&>, void>) {
            func_(ctx);
            return Result::Success;
        }
        else {
            return func_(ctx);
        }
    }

private:
    std::string name_;
    std::string description_;
    Func func_;
};

template <typename Func>
std::unique_ptr<ICommand> make_command(const std::string& name, const std::string& description, Func&& func)
{
    return std::make_unique<LambdaCommand<std::decay_t<Func>>>(
        std::move(name), 
        std::move(description),
        std::forward<Func>(func)
    );
}
}
