#pragma once

#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

class Config {
public:
    Config() { default_config(); }

    ~Config() = default;

    void default_config()
    {
        m_host.port = 16999;
        m_server.host = "50.17.206.161";
        m_server.game_version = "4.04";
        m_server.protocol = 0;
        m_command.prefix = "!";
        m_misc.force_update_game_version = false;
    }

    bool create(const std::string& file)
    {
        nlohmann::json j{};
        j["host"]["port"] = m_host.port;
        j["server"]["host"] = m_server.host;
        j["server"]["gameVersion"] = m_server.game_version;
        j["server"]["protocol"] = m_server.protocol;
        j["command"]["prefix"] = m_command.prefix;
        j["misc"]["forceUpdateGameVersion"] = m_misc.force_update_game_version;

        std::ofstream ofs{ file };
        if (!ofs.is_open()) {
            spdlog::error("Failed to open config file.");
            return false;
        }

        ofs << std::setw(4) << j;
        ofs.close();
        return true;
    }

    bool load(const std::string& file)
    {
        std::ifstream ifs{ file };
        if (!ifs.is_open()) {
            return create(file);
        }

        nlohmann::json j{};
        ifs >> j;
        ifs.close();

        try {
            m_host.port = j["host"]["port"].get<std::uint16_t>();
            m_server.host = j["server"]["host"];
            m_server.game_version = j["server"]["gameVersion"];
            m_server.protocol = j["server"]["protocol"];
            m_command.prefix = j["command"]["prefix"];
            m_misc.force_update_game_version = j["misc"]["forceUpdateGameVersion"];
        }
        catch (const nlohmann::json::exception& ex) {
            spdlog::error("{}", ex.what());
            if (ex.id != 302) { // ignore null data
                return false;
            }

            spdlog::error("Please delete current config.json to fix null config.");
        }

        return true;
    }

public:
    struct {
        std::uint16_t port;
    } m_host;

    struct {
        std::string host;
        std::string game_version;
        int protocol;
        bool using_new_packet;
    } m_server;

    struct {
        std::string prefix;
    } m_command;

    struct {
        bool force_update_game_version;
    } m_misc;
};
