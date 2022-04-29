#pragma once
#include <string>
#include <vector>

namespace utils {
    class TextParse {
    public:
        TextParse() = default;
        ~TextParse() = default;

        explicit TextParse(const std::string &string) {
            m_data = string_tokenize(string, "\n");
            for (auto &data : m_data) {
                std::replace(data.begin(), data.end(), '\r', '\0');
            }
        };

        static std::vector<std::string> string_tokenize(const std::string &string, const std::string &delimiter = "|") {
            std::vector<std::string> tokens;
            std::string::size_type last_pos = string.find_first_not_of(delimiter, 0);
            std::string::size_type pos = string.find_first_of(delimiter, last_pos);

            while (pos != std::string::npos || last_pos != std::string::npos) {
                tokens.push_back(string.substr(last_pos, pos - last_pos));
                last_pos = string.find_first_not_of(delimiter, pos);
                pos = string.find_first_of(delimiter, last_pos);
            }
            return tokens;
        }

        std::string get(const std::string &key, int index, const std::string &token = "|", int key_index = 0) {
            if (m_data.empty()) {
                return "";
            }

            for (auto &data : m_data) {
                if (data.empty()) {
                    continue;
                }

                std::vector<std::string> tokenize = string_tokenize(data, token);
                if (tokenize[key_index] == key) {
                    if (index < 0 || index >= tokenize.size()) {
                        return "";
                    }

                    // Found it.
                    return tokenize[key_index + index];
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

            for (auto &data : m_data) {
                std::vector<std::string> tokenize = string_tokenize(data, token);
                if (tokenize[0] == key) {
                    data = std::string{ tokenize[0] };
                    data += token;
                    data += value;
                    break;
                }
            }
        }

        template<typename T, typename std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool> = true>
        void set(const std::string &key, const T &value, const std::string &token = "|") {
            set(key, std::to_string(value), token);
        }

        std::vector<std::string> get_all_array() {
            std::vector<std::string> ret{};
            for(int i = 0; i < m_data.size(); i++)
                ret.push_back(fmt::format("[{}]: {}", i, m_data[i]));
            return ret;
        }
        std::string get_all_raw() {
            std::string string{};
            for (int i = 0; i < m_data.size(); i++) {
                string += m_data.at(i);
                if (i + 1 >= m_data.size())
                    continue;
                if (!m_data.at(i + 1).empty())
                    string += '\n';
            }
            return string;
        }

        bool empty() {
            return m_data.empty();
        }
        size_t get_line_count() {
            return m_data.size();
        }

    private:
        std::vector<std::string> m_data;
    };
}
