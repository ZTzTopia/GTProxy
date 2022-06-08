#pragma once
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>

#define GROWTOPIA_HOST "52.44.105.194"
#define GROWTOPIA_VERSION "3.91"
#define GROWTOPIA_PROTOCOL 161

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
            json["server"]["host"] = GROWTOPIA_HOST; // growtopia1.com
            json["server"]["gameVersion"] = GROWTOPIA_VERSION;
            json["server"]["protocol"] = GROWTOPIA_PROTOCOL;
            json["server"]["usingNewPacket"] = true;
            json["command"]["prefix"] = "!";

            std::ofstream out(path);
            out << json.dump(4);
            out.close();
        }
        else {
            in >> json;
            bool need_to_save{ false };

            if (json["server"].contains("game_version")) {
                json["server"].erase("game_version");
                json["server"]["gameVersion"] = GROWTOPIA_VERSION;
                need_to_save = true;
            }

            if (!json["server"].contains("usingNewPacket")) {
                json["server"]["usingNewPacket"] = true;
                need_to_save = true;
            }

			if (json["server"].contains("host")) {
                if (json["server"]["host"] == "52.86.208.1")
                    json["server"]["host"] = GROWTOPIA_HOST;
                need_to_save = true;
            }

            if (!json.contains("command")) {
                if (!json["command"].contains("prefix"))
                    json["command"]["prefix"] = "!";
                need_to_save = true;
            }

            if (need_to_save) {
                std::ofstream out{ path };
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

#undef GROWTOPIA_HOST
#undef GROWTOPIA_VERSION
#undef GROWTOPIA_PROTOCOL
