#include <future>
#include <chrono>
#include <enet/enet.h>
#include <spdlog/spdlog.h>

#include "core.hpp"
#include "../client/client.hpp"
#include "../server/server.hpp"

namespace core {
Core::Core()
    : run_{ true }
    , tick_{ 0 }
{
    if (enet_initialize() != 0) {
        throw std::runtime_error{ "Failed to initialize ENet" };
    }

    server_ = new server::Server{ this };
    client_ = new client::Client{ this };
}

Core::~Core()
{
    delete client_;
    delete server_;
    enet_deinitialize();
}

bool Core::add_extension(extension::IExtension* ext)
{
    spdlog::debug("Checking if extension with UID 0x{:x} should be ignored", ext->get_uid());
    for (const auto& ignore_uid : config_.get<std::vector<std::string>>("extension.ignore")) {
        if (ext->get_uid() == std::stoull(ignore_uid, nullptr, 16)) {
            spdlog::info("Ignoring extension with UID 0x{:x}", ext->get_uid());
            return false;
        }
    }

    return extension::Extensible::add_extension(ext);
}

void Core::run()
{
    event_dispatcher_.dispatch(EventInit{});
    for (const auto& ext : std::views::values(extensions_)) {
        ext->init();
    }

    constexpr std::chrono::microseconds sleep_timer{ static_cast<int>(5.0f * 1000.0f) };
    auto prev{ std::chrono::high_resolution_clock::now() };
    std::chrono::microseconds sleep_duration{ sleep_timer };

    while (run_) {
        const auto now{ std::chrono::high_resolution_clock::now() };
        // Elapsed time since previous iteration
        const auto us{ std::chrono::duration_cast<std::chrono::microseconds>(now - prev) };
        prev = now;

        // Adjust the sleep duration based on the time spent processing
        sleep_duration += sleep_timer - us;

        // Use std::async to run server and client processing asynchronously
        auto server_future{ std::async(std::launch::async, [this] { server_->process(); }) };
        auto client_future{ std::async(std::launch::async, [this] { client_->process(); }) };

        // Wait for both tasks to complete
        server_future.get();
        client_future.get();

        // Call the tick callback
        event_dispatcher_.dispatch(EventTick{}); // TODO: Pass tick related arguments to the callback
        for (const auto& ext : std::views::values(extensions_)) {
            ext->tick();
        }

        if (sleep_duration > std::chrono::microseconds::zero()) {
            std::this_thread::sleep_for(sleep_duration);
        }

        // Reset sleep_duration if it's greater than sleep_timer
        if (sleep_duration >= sleep_timer) {
            sleep_duration = std::chrono::microseconds::zero();
        }

        tick_++;
    }
}
}
