#pragma once
#include <string>
#include <vector>

class TextParse {
public:
    TextParse() = default;
    ~TextParse() = default;

    explicit TextParse(const std::string &data) {
        m_data = string_tokenize(data, "\n");
        for (auto & i : m_data) {
            std::replace(i.begin(), i.end(), '\t', '\0');
        }
    };

    static std::vector<std::string> string_tokenize(const std::string &string, const std::string &delimiters) {
        std::vector<std::string> tokens;
        std::string::size_type last_pos = string.find_first_not_of(delimiters, 0);
        std::string::size_type pos = string.find_first_of(delimiters, last_pos);

        while (std::string::npos != pos || std::string::npos != last_pos) {
            tokens.push_back(string.substr(last_pos, pos - last_pos));
            last_pos = string.find_first_not_of(delimiters, pos);
            pos = string.find_first_of(delimiters, last_pos);
        }

        return tokens;
    }

    std::string get(const std::string &key, int index, const std::string &token = "|") {
        if (m_data.empty()) {
            return "";
        }

        for (auto &i : m_data) {
            if (i.empty()) {
                continue;
            }

            std::vector<std::string> data = string_tokenize(i, token);
            if (data[0] == key) {
                // TODO: Fix crash.
                if (index < 0 || index >= data.size()) {
                    return "";
                }

                // Found it.
                return data[index];
            }
        }

        return "";
    }

    template<typename T, typename std::enable_if_t<std::is_integral_v<T>, bool> = true>
    T get(const std::string &key, int index, const std::string &token = "|") {
        return std::stoi(get(key, index, token));
    }

    template<typename T, typename std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    T get(const std::string &key, int index, const std::string &token = "|") {
        if (std::is_same_v<T, double>) {
            return std::stod(get(key, index, token));
        }
        else if (std::is_same_v<T, long double>) {
            return std::stold(get(key, index, token));
        }

        return std::stof(get(key, index, token));
    }

    void add(const std::string &key, const std::string &value, const std::string &token = "|") {
        std::string data = key + token + value;
        m_data.push_back(data);
    }

    template<typename T, typename std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool> = true>
    void add(const std::string &key, const T &value, const std::string &token = "|") {
        add(key, std::to_string(value), token);
    }

    void set(const std::string &key, const std::string &value, const std::string &token = "|") {
        if (m_data.empty()) {
            return;
        }

        for (auto &i : m_data) {
            std::vector<std::string> data = string_tokenize(i, token);
            if (data[0] == key) {
                i = data[0];
                i += token;
                i += value;
                break;
            }
        }
    }

    template<typename T, typename std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool> = true>
    void set(const std::string &key, const T &value, const std::string &token = "|") {
        set_(key, std::to_string(value), token);
    }

    void get_all_raw(std::string &data) {
        if (m_data.empty()) {
            return;
        }

        std::string string{};
        for (unsigned int i = 0; i < m_data.size(); i++) {
            string += m_data[i];
            if (i + 1 < m_data.size() && !m_data[i + 1].empty()) {
                string += "\n";
            }
        }

        data = string;
    }

    size_t get_line_count() {
        return m_data.size();
    }

private:
    std::vector<std::string> m_data;
};