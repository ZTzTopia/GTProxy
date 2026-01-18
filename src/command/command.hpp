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
    core::Scheduler& scheduler;
};

class ICommand {
public:
    virtual ~ICommand() = default;

    [[nodiscard]] virtual std::string_view name() const = 0;

    virtual Result execute(const Context& ctx) = 0;
};

template <typename Func>
class LambdaCommand final : public ICommand {
public:
    LambdaCommand(std::string  name, Func func)
        : name_{ std::move(name) }
        , func_{ std::move(func) }
    { }

    [[nodiscard]] std::string_view name() const override { return name_; }

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
    Func func_;
};

template <typename Func>
std::unique_ptr<ICommand> make_command(const std::string& name, Func&& func)
{
    return std::make_unique<LambdaCommand<std::decay_t<Func>>>(
        std::move(name), 
        std::forward<Func>(func)
    );
}
}
