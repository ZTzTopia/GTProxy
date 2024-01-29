#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <utility>

namespace core {
class Logger {
public:
    Logger() = default;
    ~Logger() { logger_->flush(); }

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
            1024 * 1024 * 2,
            4
        );
    }

    [[nodiscard]] std::shared_ptr<spdlog::logger> get_logger() const { return logger_; }
    void set_logger(std::shared_ptr<spdlog::logger> logger) { logger_ = std::move(logger); }

private:
    std::shared_ptr<spdlog::logger> logger_;
};
}
