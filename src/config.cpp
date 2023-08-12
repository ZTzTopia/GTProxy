#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "config.h"

Config::Config()
    : m_host{}
    , m_server{}
    , m_command{}
    , m_misc{}
{
    // Initializes the configuration settings to default values.
    m_host.m_port = 16999;
    m_server.m_host = "www.growtopia1.com";
    m_server.m_game_version = "4.34";
    m_server.m_protocol = 191;
    m_server.m_platform_id = "4";
    m_command.m_prefix = "!";
    m_misc.m_force_update_game_version = false;
    m_misc.m_force_update_protocol = false;
}

bool Config::create(const std::string& file)
{
    nlohmann::json j{};
    j["host"]["port"] = m_host.m_port;
    j["server"]["host"] = m_server.m_host;
    j["server"]["gameVersion"] = m_server.m_game_version;
    j["server"]["protocol"] = m_server.m_protocol;
    j["server"]["platformID"] = m_server.m_platform_id;
    j["command"]["prefix"] = m_command.m_prefix;
    j["misc"]["forceUpdateGameVersion"] = m_misc.m_force_update_game_version;
    j["misc"]["forceUpdateGameVersionProtocol"] = m_misc.m_force_update_protocol;

    std::ofstream ofs{ file };
    if (!ofs.is_open()) {
        spdlog::error("Failed to open config file.");
        return false;
    }

    ofs << std::setw(4) << j;
    return true;
}

bool Config::load(const std::string& file)
{
    std::ifstream ifs{ file };
    if (!ifs.is_open()) {
        return create(file);
    }

    nlohmann::json j{};
    ifs >> j;

    try {
        m_host.m_port = j["host"]["port"].get<std::uint16_t>();
        m_server.m_host = j["server"]["host"];
        m_server.m_game_version = j["server"]["gameVersion"];
        m_server.m_protocol = j["server"]["protocol"].get<std::uint8_t>();
        m_server.m_platform_id = j["server"]["platformID"];
        m_command.m_prefix = j["command"]["prefix"];
        m_misc.m_force_update_game_version = j["misc"]["forceUpdateGameVersion"];
        m_misc.m_force_update_protocol = j["misc"]["forceUpdateGameVersionProtocol"];
    }
    catch (const nlohmann::json::exception& ex) {
        if (ex.id != 302) {
            spdlog::error("Configuration file \"{}\" failed to load.", file);
            spdlog::error("{}", ex.what());
            return false;
        }

        spdlog::warn("The configuration file \"{}\" is empty or contains a null value.", file);
        create(file);
    }

    spdlog::info("Configuration file \"{}\" loaded successfully.", file);
    return true;
}
