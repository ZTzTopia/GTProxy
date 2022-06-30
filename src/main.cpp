#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "config.h"
#include "enetwrapper/enet_wrapper.h"
#include "server/server.h"
#include "utils/text_parse.h"

int main()
{
    {
        std::srand(static_cast<unsigned int>(std::time(nullptr)) + std::clock());

        try {
            std::vector<spdlog::sink_ptr> sinks;
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
            sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>("proxy.log", 1024 * 1024 * 5, 16));

            std::shared_ptr<spdlog::logger> combined_logger{ std::make_shared<spdlog::logger>("GTProxy", sinks.begin(), sinks.end()) };
            combined_logger->set_level(spdlog::level::trace);

            spdlog::register_logger(combined_logger);
            spdlog::set_default_logger(combined_logger);
        }
        catch (const spdlog::spdlog_ex& ex) {
            std::cout << "Log initialization failed: " << ex.what() << std::endl;
            return 1;
        }

        spdlog::info("Starting Growtopia proxy v{}...", GTPROXY_VERSION);
        if (!enetwrapper::ENetWrapper::one_time_init()) {
            spdlog::error("Failed to initialize ENet server.");
            return 1;
        }

        auto config{ new Config{} };
        if (!config->load("config.json")) {
            return 1;
        }

        auto server{ new server::Server{ config } };
        if (!server->start()) {
            spdlog::error("Failed to start proxy server.");
            return 1;
        }

        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    return 0;
}
