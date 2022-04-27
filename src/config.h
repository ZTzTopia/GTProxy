#pragma once
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>

class Config {
public:
    static Config& get() {
        static Config config;
        return config;
    }

    void load(const std::string& path) {
        nlohmann::json json;
        std::ifstream file(path);

        if (!file.is_open()) {
            json["server"]["host"] = "http://13.248.211.25"; // https://growtopia1.com
            json["server"]["game_version"] = "3.86";
            json["server"]["protocol"] = 160;
            m_config = json;
            std::ofstream out(path);
            out << json.dump(4);
            return;
        }

        file >> json;
        file.close();
        m_config = json;
    }

    nlohmann::json& config() {
        return m_config;
    }

private:
    nlohmann::json m_config;
};