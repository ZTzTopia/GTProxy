#include <cstdlib>
#include <ctime>
#include <iostream>
#include <httplib/httplib.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "config.h"
#include "enetwrapper/enetwrapper.h"
#include "server/server.h"
#include "utils/text_parse.h"

int main()
{
    {
        srand(static_cast<unsigned int>(std::time(nullptr)) + std::clock());

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

        Config::get().load("./config.json");

        if (!enetwrapper::ENetWrapper::one_time_init()) {
            spdlog::error("Failed to initialize ENet server.");
            return 1;
        }

        auto proxy_server{ std::make_unique<server::Server>() };
        if (!proxy_server->initialize()) {
            spdlog::error("Failed to initialize proxy server.");
            return 1;
        }

        httplib::Server svr{};
        svr.Post("/growtopia/server_data.php", [](const auto& req, auto& res) {
            if (!req.body.empty())
                spdlog::info("HTTP Request body: {}", req.body);

            httplib::Client cli{ Config::get().config()["server"]["host"] };
            httplib::Result response{ cli.Post("/growtopia/server_data.php") };
            if (response.error() != httplib::Error::Success || response->status != 200) {
                if (response.error() == httplib::Error::Success)
                    spdlog::error("Failed to get server data. HTTP status code: {}", response->status);
                else
                    spdlog::error("Failed to get server data. HTTP error: {}", httplib::to_string(response.error()));

                return false;
            }

            utils::TextParse text_parse{ response->body };
            text_parse.set("server", "127.0.0.1");
            text_parse.set("port", "17000");

            res.set_content(text_parse.get_all_raw(), "text/html");
            return true;
        });

        spdlog::info("HTTP Server listening to {}:{}", "0.0.0.0", 80);
        svr.listen("0.0.0.0", 80);
    }

    return 0;
}
