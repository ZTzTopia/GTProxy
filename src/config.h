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
        nlohmann::json json{};

        std::ifstream in{ path };
        if (!in.is_open()) {
            json["server"]["host"] = "http://13.248.211.25"; // http://growtopia1.com
            json["server"]["game_version"] = "3.86";
            json["server"]["protocol"] = 160;
            json["command"]["prefix"] = "!";

            std::ofstream out(path);
            out << json.dump(4);
            out.close();
        }
        else {
            in >> json;

            if (!json.contains("command")) {
                if (!json["command"].contains("prefix"))
                    json["command"]["prefix"] = "!";

                std::ofstream out(path);
                out << json.dump(4);
                out.close();
            }

            in.close();
        }

        m_config = json;
    }

    nlohmann::json& config() {
        return m_config;
    }

private:
    nlohmann::json m_config;
};