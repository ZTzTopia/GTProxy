#include "core/core.hpp"
#include "core/logger.hpp"

#include "extension/parser/parser_impl.hpp"
#include "extension/sub_server_switch/sub_server_switch_impl.hpp"
#include "extension/web_server/web_server_impl.hpp"
#include "extension/command_handler/command_handler_impl.hpp"

int main()
{
    try {
        core::Logger logger{};

        std::vector<spdlog::sink_ptr> sinks{
            core::Logger::create_console_sink(),
            core::Logger::create_file_sink()
        };

        logger.set_logger(std::make_shared<spdlog::logger>("GTProxy", sinks.begin(), sinks.end()));
        logger.get_logger()->set_level(spdlog::level::trace);

        spdlog::register_logger(logger.get_logger());
        spdlog::set_default_logger(logger.get_logger());
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [GTProxy] [%^%l%$] %v");
    }
    catch (const spdlog::spdlog_ex& ex) {
        spdlog::error("Log initialization failed: {}", ex.what());
        return 1;
    }

    try {
        spdlog::info(
            "Starting GTProxy (v{}.{}.{})",
            GTPROXY_VERSION_MAJOR,
            GTPROXY_VERSION_MINOR,
            GTPROXY_VERSION_PATCH
        );

        core::Core core{};

        /**
         * Register event listeners and handle events by adding extensions to the core
         *
         * Extensions serve as the primary mechanism to enhance the core's functionality.
         * They allow the addition of new features, such as a web server or a parser.
         *
         * This process involves using the dispatch pattern to listen for events emitted by the core.
         */
        core.add_extension(new extension::web_server::WebServerExtension{ &core });
        core.add_extension(new extension::parser::ParserExtension{ &core });
        core.add_extension(new extension::sub_server_switch::SubServerSwitchExtension{ &core });
        core.add_extension(new extension::command_handler::CommandHandlerExtension{ &core });

        // Run the core (Will block the main thread until the core is stopped)
        core.run();
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
