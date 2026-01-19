#include "core.hpp"

#include <chrono>
#include <thread>
#include <enet/enet.h>
#include <spdlog/spdlog.h>

#include "../packet/register_packets.hpp"
#include "../scripting/bindings/command_bindings.hpp"
#include "../scripting/bindings/logger_bindings.hpp"
#include "../scripting/bindings/packet_bindings.hpp"

namespace core {
Core::Core()
    : running_{ true }
    , scheduler_{ std::make_shared<Scheduler>() }
{
    if (enet_initialize() != 0) {
        throw std::runtime_error{ "Failed to initialize ENet" };
    }

    server_ = std::make_unique<network::Server>(config_, dispatcher_);
    client_ = std::make_unique<network::Client>(config_, dispatcher_);
    web_server_ = std::make_unique<WebServer>(config_, dispatcher_, *client_, *server_);

    packet::register_all_packets();

    session_handler_ = std::make_unique<SessionHandler>(config_, dispatcher_, *client_, *server_);
    command_handler_ = std::make_unique<command::CommandHandler>(config_, dispatcher_, scheduler_, *server_, *client_);

    script_engine_ = std::make_unique<scripting::LuaEngine>();

    script_engine_->register_binding(std::make_unique<scripting::bindings::LoggerBindings>());
    script_engine_->register_binding(std::make_unique<scripting::bindings::CommandBindings>(*command_handler_));

    script_loader_ = std::make_unique<scripting::ScriptLoader>(*script_engine_, "scripts");
    script_loader_->load_all();

    spdlog::info("Core initialized successfully");
}

Core::~Core()
{
    enet_deinitialize();
}

void Core::run() const
{
    constexpr auto sleep_timer = std::chrono::microseconds{ 5000 };
    auto prev = std::chrono::high_resolution_clock::now();
    auto sleep_duration = sleep_timer;

    while (running_) {
        const auto now = std::chrono::high_resolution_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - prev);
        prev = now;

        sleep_duration += sleep_timer - elapsed;

        server_->process();
        client_->process();

        if (sleep_duration > std::chrono::microseconds::zero()) {
            std::this_thread::sleep_for(sleep_duration);
        }

        if (sleep_duration >= sleep_timer) {
            sleep_duration = std::chrono::microseconds::zero();
        }
    }
}
}
