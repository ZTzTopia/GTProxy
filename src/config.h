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
            json["server"]["gameVersion"] = "3.87";
            json["server"]["protocol"] = 161;
            json["server"]["usingNewPacket"] = true;
            json["command"]["prefix"] = "!";

            std::ofstream out(path);
            out << json.dump(4);
            out.close();
        }
        else {
            in >> json;
            bool need_to_save{ false };

            if (!json["server"].contains("usingNewPacket")) {
                json["server"]["usingNewPacket"] = true;
                need_to_save = true;
            }

            if (json["server"].contains("game_version")) {
                json["server"].erase("game_version");
                json["server"]["gameVersion"] = "3.86";
                need_to_save = true;
            }

            if (!json.contains("command")) {
                if (!json["command"].contains("prefix"))
                    json["command"]["prefix"] = "!";
                need_to_save = true;
            }

            if (need_to_save) {
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