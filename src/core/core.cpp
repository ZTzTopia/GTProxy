#include <future>
#include <chrono>
#include <enet/enet.h>

#include "core.hpp"
#include "../client/client.hpp"
#include "../server/server.hpp"

namespace core {
Core::Core()
    : config_{}
    , run_{ true }
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

void Core::run()
{
    const std::chrono::microseconds sleep_timer{ static_cast<int>(5.0 * 1000.0) };
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
        auto server_future{ std::async(std::launch::async, [this]() { server_->process(); }) };
        auto client_future{ std::async(std::launch::async, [this]() { client_->process(); }) };

        // Wait for both tasks to complete
        server_future.get();
        client_future.get();

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
