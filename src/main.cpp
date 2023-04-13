#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "config.h"
#include "enetwrapper/enet_wrapper.h"
#include "server/server.h"

int main()
{
    try {
        std::vector<spdlog::sink_ptr> sinks{};

        // Add a stdout sink and a rotating file sink to the vector of sink pointers.
        // The rotating file sink rotates the log file every 5 MB and keeps up to 16 rotated files.
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        sinks.push_back(
            std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                "proxy.log",
                1024 * 1024 * 5,
                16
            )
        );

        auto combined_logger{
            std::make_shared<spdlog::logger>(
                "GTProxy",
                sinks.begin(),
                sinks.end()
            )
        };

#ifdef GTPROXY_DEBUG
        combined_logger->set_level(spdlog::level::trace);
#else
        combined_logger->set_level(spdlog::level::info);
#endif

        spdlog::register_logger(combined_logger);
        spdlog::set_default_logger(combined_logger);
        spdlog::flush_on(spdlog::level::debug);
    }
    catch (const spdlog::spdlog_ex& ex) {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
        return 1;
    }

    spdlog::info("Starting GTProxy v{}...", GTPROXY_VERSION);

    auto config{ new Config{} };
    if (!config->load("config.json")) {
        return 1;
    }

    if (!enet_wrapper::ENetWrapper::one_time_init()) {
        spdlog::error("Failed to initialize ENet server.");
        return 1;
    }

    auto server{ std::make_unique<server::Server>(config) };
    if (!server->start()) {
        return 1;
    }

    while (server.get()) { // Just to avoid compiler warning.
        std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
    }

    return 0;
}
