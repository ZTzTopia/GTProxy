#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace core {
class Logger {
public:
    Logger()
    {
        std::vector<spdlog::sink_ptr> sinks{
            create_console_sink(),
            create_file_sink()
        };

        logger_ = std::make_shared<spdlog::logger>(
            "GTProxy",
            sinks.begin(),
            sinks.end()
        );
        logger_->set_level(spdlog::level::trace);

        spdlog::register_logger(logger_);
        spdlog::set_default_logger(logger_);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [GTProxy] [%^%l%$] %v");
    }

    ~Logger() { logger_->flush(); }

private:
    [[nodiscard]] static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> create_console_sink()
    {
        auto console_sink{ std::make_shared<spdlog::sinks::stdout_color_sink_mt>() };
#ifdef GTPROXY_DEBUG
        console_sink->set_level(spdlog::level::trace);
#else
        console_sink->set_level(spdlog::level::info);
#endif
        return console_sink;
    }

    [[nodiscard]] static std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> create_file_sink()
    {
        return std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "proxy.log",
            1024 * 1024 * 5,
            16
        );
    }

private:
    std::shared_ptr<spdlog::logger> logger_;
};
}
