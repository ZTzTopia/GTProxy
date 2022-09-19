#pragma once
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

class Config {
public:
    Config() : m_server{}, m_command{}, m_ssl{} {}
    ~Config() = default;

    void default_config()
    {
        m_server.host = "growtopia1.com";
        m_server.game_version = "3.99";
        m_server.protocol = 173;
        m_server.using_new_packet = true;
        m_command.prefix = "!";
    }

    bool create(const std::string& file)
    {
        default_config();

        nlohmann::json j{};
        j["server"]["host"] = m_server.host;
        j["server"]["gameVersion"] = m_server.game_version;
        j["server"]["protocol"] = m_server.protocol;
        j["server"]["usingNewPacket"] = m_server.using_new_packet;
        j["command"]["prefix"] = m_command.prefix;
        j["ssl"]["enabled"] = m_ssl.enabled;

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
            m_server.host = j["server"]["host"];
            m_server.game_version = j["server"]["gameVersion"];
            m_server.protocol = j["server"]["protocol"].get<int>();
            m_server.using_new_packet = j["server"]["usingNewPacket"];
            m_command.prefix = j["command"]["prefix"];
            m_ssl.enabled = j["ssl"]["enabled"];
        }
        catch (const nlohmann::json::exception& ex) {
            spdlog::error("{}", ex.what());
            return false;
        }

        return true;
    }

public:
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
        bool enabled;
    } m_ssl;
};
