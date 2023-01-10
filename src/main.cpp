#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "config.h"
#include "server/server.h"
#include "utils/text_parse.h"

int main()
{
    try {
        std::vector<spdlog::sink_ptr> sinks;
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>("proxy.log", 1024 * 1024 * 5, 16));

        auto combined_logger{ std::make_shared<spdlog::logger>("GTProxy", sinks.begin(), sinks.end()) };
        combined_logger->set_level(spdlog::level::trace);

        spdlog::register_logger(combined_logger);
        spdlog::set_default_logger(combined_logger);
    }
    catch (const spdlog::spdlog_ex& ex) {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
        return 1;
    }

    spdlog::info("Starting Growtopia proxy v{}...", GTPROXY_VERSION);

    auto config{ new Config{}};
    if (!config->load("config.json")) {
        return 1;
    }

    auto server{ std::make_unique<server::Server>(config) };
    if (!server->start()) {
        return 1;
    }

    while (server.get()) { // Just to avoid compiler warning.
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
