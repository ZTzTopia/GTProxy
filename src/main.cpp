#include "core/core.hpp"
#include "core/logger.hpp"
#include "extension/parser/parser_impl.hpp"
#include "extension/web_server/web_server_impl.hpp"

int main()
{
    try {
        core::Logger logger{};

        std::vector<spdlog::sink_ptr> sinks{
            core::Logger::create_console_sink(),
            core::Logger::create_file_sink()
        };

        logger.set_logger(
            std::make_shared<spdlog::logger>(
                "GTProxy",
                sinks.begin(),
                sinks.end()
            )
        );
        logger.get_logger()->set_level(spdlog::level::trace);

        spdlog::register_logger(logger.get_logger());
        spdlog::set_default_logger(logger.get_logger());
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [GTProxy] [%^%l%$] %v");

        spdlog::info(
            "Starting GTProxy (v{}.{}.{})",
            GTPROXY_VERSION_MAJOR,
            GTPROXY_VERSION_MINOR,
            GTPROXY_VERSION_PATCH
        );

        core::Core core{};
        core.add_extension(new extension::web_server::WebServerExtension{ &core });
        core.add_extension(new extension::parser::ParserExtension{ &core });
        core.run();
    }
    catch (const spdlog::spdlog_ex& ex) {
        spdlog::error("Log initialization failed: {}", ex.what());
        return 1;
    }
    catch (const std::runtime_error& e) {
        spdlog::error("Runtime error: {}", e.what());
        return 1;
    }
    catch (const std::exception& ex) {
        spdlog::error("Exception: {}", ex.what());
        return 1;
    }

    return 0;
}
