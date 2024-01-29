#pragma once
#include <string>
#include <unordered_map>
#include <variant>

namespace core {
using ConfigStorage = std::variant<int, unsigned int, std::string, bool, std::vector<std::string>>;

class Config {
public:
    Config();
    ~Config() = default;

    template <typename T = std::string>
    [[nodiscard]] T get(const std::string& key) const
    {
        try {
            return std::get<T>(config_.at(key));
        }
        catch (const std::exception&) {
            return T{}; // or some other default value
        }
    }

private:
    std::unordered_map<std::string, ConfigStorage> config_;
};
}
