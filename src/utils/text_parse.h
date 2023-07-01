#pragma once
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace utils {
class TextParse {
public:
    TextParse() = delete;
    ~TextParse() = default;

    explicit TextParse(const std::string& string)
        : m_data{ string_tokenize(string, "\n") }
    {
       // for (auto& data : m_data) {
       //     data.erase(std::remove(data.begin(), data.end(), '\r'), data.end());
       //     std::replace(data.begin(), data.end(), '\r', '\0');
       // }
    }

    static std::vector<std::string> string_tokenize(
        const std::string& string,
        const std::string_view& delimiter = "|"
    ) {
        std::vector<std::string> tokens{};
        for (auto&& token : string | std::views::split(delimiter)) {
            if (token.empty()) {
                continue;
            }

            tokens.emplace_back(token.begin(), token.end());
        }

        return tokens;
    }

    std::string get(const std::string& key, int index, const std::string_view& token = "|", int key_index = 0)
    {
        if (m_data.empty()) {
            return {};
        }

        for (auto& data : m_data) {
            if (data.empty()) {
                continue;
            }

            std::vector<std::string> tokenize = string_tokenize(data, token);
            if (tokenize[key_index] == key) {
                if (index < 0 || index >= tokenize.size()) {
                    return {};
                }

                return tokenize[key_index + index];
            }
        }

        return {};
    }

    template <typename T, typename std::enable_if_t<std::is_signed_v<T>, bool> = true>
    T get(const std::string& key, int index, const std::string_view& token = "|")
    {
        return std::stoi(get(key, index, token));
    }

    template <typename T, typename std::enable_if_t<std::is_unsigned_v<T>, bool> = true>
    T get(const std::string& key, int index, const std::string_view& token = "|")
    {
        return std::stoul(get(key, index, token));
    }

    template <typename T, typename std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    T get(const std::string& key, int index, const std::string_view& token = "|")
    {
        if (std::is_same_v<T, double>) {
            return std::stod(get(key, index, token));
        }
        else if (std::is_same_v<T, long double>) {
            return std::stold(get(key, index, token));
        }

        return std::stof(get(key, index, token));
    }

    void add(const std::string& key, const std::string& value, const std::string_view& token = "|")
    {
        std::string data = key + std::string{ token } + value;
        m_data.push_back(data);
    }

    template <typename T, typename std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool> = true>
    void add(const std::string& key, const T& value, const std::string_view& token = "|")
    {
        add(key, std::to_string(value), token);
    }

    void set(const std::string& key, const std::string& value, const std::string_view& token = "|")
    {
        if (m_data.empty())
            return;

        for (auto& data : m_data) {
            std::vector<std::string> tokenize = string_tokenize(data, token);
            if (tokenize[0] == key) {
                data = std::string{ tokenize[0] };
                data += token;
                data += value;
                break;
            }
        }
    }

    template <typename T, typename std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool> = true>
    void set(const std::string& key, const T& value, const std::string_view& token = "|")
    {
        set(key, std::to_string(value), token);
    }

    void remove(const std::string &key, const std::string_view &token = "|")
    {
        if (m_data.empty()) {
            return;
        }

        const auto first_token = key + std::string{ token };
        m_data.erase(std::remove_if(m_data.begin(), m_data.end(), [&](const std::string& element) {
            return element.compare(0, first_token.length(), first_token) == 0;
        }), m_data.end());
    }

    std::vector<std::string> get_all_array()
    {
        std::vector<std::string> ret{};
        for (int i = 0; i < m_data.size(); i++) {
            ret.push_back(fmt::format("[{}]: {}", i, m_data[i]));
        }

        return ret;
    }

    std::string get_all_raw()
    {
        std::string string{};
        for (auto it = m_data.cbegin(); it != m_data.cend(); ++it) {
            string += *it;
            // if (std::next(it) != m_data.cend() && !std::next(it)->empty()) {
                string += "\n";
            // }
        }

        return string;
    }

    // won't add if key already exist.
    // return true if the key doesn't exist yet, otherwise false.
    bool add_key_once(const std::string& key)
    {
        auto it = std::find_if(m_data.begin(), m_data.end(), [&](const std::string& str) -> bool
        {
            return str.find(key) != std::string::npos;
        });
        if (it == m_data.end()) {
            add_key(key);
            return true;
        }
        return false;
    }

    void add_key(const char* c_str)
    {
        std::string str {c_str};
        add_key(str);
    }

    void add_key(const std::string &key) {
        m_data.push_back(key);
    }

public:
    bool empty() { return m_data.empty(); }
    std::size_t size() { return m_data.size(); }

private:
    std::vector<std::string> m_data;
};
}
