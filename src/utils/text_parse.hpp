#pragma once
#include <ranges>
#include <string>
#include <vector>
#include <unordered_map>

class TextParse {
public:
    TextParse() = default;

    explicit TextParse(const std::string& str, const std::string& delimiter = "|")
    {
        for (const auto& line : tokenize(str, "\n")) {
            std::vector tokens{ tokenize(line, delimiter) };
            std::string key{ tokens.front() };

            tokens.erase(tokens.begin());
            tokens.shrink_to_fit();

            m_data.emplace(key, tokens);
        }
    }

    static std::vector<std::string> tokenize(const std::string& string, const std::string& delimiter = "|")
    {
        std::vector<std::string> tokens{};
        for (auto&& token : string | std::views::split(delimiter)) {
            if (token.empty()) {
                continue;
            }

            tokens.emplace_back(token.begin(), token.end());
        }

        return tokens;
    }

    [[nodiscard]] std::string get(const std::string& key, const int index = 0) const
    {
        const auto it{ m_data.find(key) };
        if (it == m_data.end()) {
            return {};
        }

        if (index < 0 || index >= it->second.size()) {
            return {};
        }

        return it->second[index];
    }

    template <typename T>
    [[nodiscard]] T get(const std::string& key, const int index = 0) const
    {
        if constexpr (std::is_integral_v<T>) {
            if constexpr (std::is_unsigned_v<T>) {
                return std::stoul(get(key, index));
            }
            else {
                return std::stoi(get(key, index));
            }
        }
        else if constexpr (std::is_same_v<T, double>) {
            return std::stod(get(key, index));
        }
        else if constexpr (std::is_same_v<T, long double>) {
            return std::stold(get(key, index));
        }
        else if constexpr (std::is_same_v<T, float>) {
            return std::stof(get(key, index));
        }

        return {};
    }

    void add(const std::string& key, const std::vector<std::string>& value)
    {
        m_data.emplace(key, value);
    }

    void set(const std::string& key, const std::vector<std::string>& value)
    {
        if (!m_data.contains(key)) {
            return;
        }

        m_data[key] = value;
    }

    void remove(const std::string& key)
    {
        const auto it{ m_data.find(key) };
        if (it == m_data.end()) {
            return;
        }

        m_data.erase(it);
    }

    [[nodiscard]] std::string get_raw(const std::string& delimiter = "|", const std::string& prepend_text = "") const
    {
        std::string raw_data{};
        for (auto it = m_data.cbegin(); it != m_data.cend(); ++it) {
            raw_data += prepend_text + it->first;
            for (const auto& token : it->second) {
                raw_data += delimiter + token;
            }

            if (std::next(it) != m_data.cend() && !std::next(it)->first.empty()) {
                raw_data += '\n';
            }
        }

        return raw_data;
    }

    [[nodiscard]] bool empty() const { return m_data.empty(); }

private:
    std::unordered_map<std::string, std::vector<std::string>> m_data;
};
