#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <httplib.h>

#include "enetwrapper/enetwrapper.h"
#include "server/server.h"

int main() {
    {
        // Initialize logger.
        std::vector<spdlog::sink_ptr> sinks;
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>("proxy.log", 1024 * 1024, 8)); // Using the port??

        auto logger = std::make_shared<spdlog::logger>("proxy", sinks.begin(), sinks.end());
        logger->set_pattern("[%Y-%m-%dT%TZ] [%n] [%^%l%$] [thread %t] %v");
        logger->set_level(spdlog::level::debug);
        logger->flush_on(spdlog::level::debug);

        spdlog::set_default_logger(logger);

        // Program logic.
        spdlog::info("Starting Growtopia proxy...");

        // Initialize enet.
        if (!enetwrapper::ENetWrapper::one_time_init()) {
            spdlog::error("Failed to initialize ENet server.");
            return 1;
        }

        // Start proxy server.
        auto proxy_server = std::make_unique<server::Server>();
        if (!proxy_server->initialize()) {
            spdlog::error("Failed to initialize proxy server.");
            return 1;
        }

        // Start http server.
        httplib::Server http_server{};
        http_server.Post("/growtopia/server_data.php", [](const httplib::Request &req, httplib::Response &res) {
            res.set_content(
                    "server|127.0.0.1\n"
                    "port|17000\n"
                    "type|1\n"
                    "#maint|Server is under maintenance. We will be back online shortly. Thank you for your patience!\n"
                    "beta_server|beta.growtopiagame.com\n"
                    "beta_port|26999\n"
                    "beta_type|1\n"
                    "beta2_server|beta2.growtopiagame.com\n"
                    "beta2_port|26999\n"
                    "beta2_type|1\n"
                    "type2|1\n"
                    "meta|defined\n"
                    "RTENDMARKERBS1001",
                "text/html");
            return true;
        });

        http_server.listen("0.0.0.0", 80);
    }
    return EXIT_SUCCESS;
}
