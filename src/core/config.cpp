#include <glaze/glaze.hpp>
#include <spdlog/spdlog.h>

#include "config.hpp"

namespace core {
Config::Config()
{
    if (const auto read_ec = glz::read_file_json(config_, "config.json", std::string{})) {
        if (read_ec.ec != glz::error_code::file_open_failure) {
            throw std::runtime_error{
                fmt::format("Failed to load configuration file: {}", glz::format_error(read_ec))
            };
        }

        spdlog::warn("Configuration file \"config.json\" not found. Creating default configuration file...");
        if (const auto write_ec = glz::write_file_json<glz::opts{.prettify = true}>(config_, "config.json", std::string{})) {
            throw std::runtime_error{
                fmt::format("Failed to create default configuration file: {}", glz::format_error(write_ec))
            };
        }

        spdlog::info("Default configuration file \"config.json\" created successfully.");
        return;
    }

    // TODO: Add missing keys when new fields are added

    spdlog::info("Config file \"config.json\" is all loaded up and ready to go!");
}
}

template<> struct glz::meta<core::Config::ServerConfig> : glz::camel_case {};
template<> struct glz::meta<core::Config::ClientConfig> : glz::camel_case {};
template<> struct glz::meta<core::Config::LogConfig> : glz::camel_case {};
template<> struct glz::meta<core::Config::WrapperConfig> : glz::camel_case {};
