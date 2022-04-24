#pragma once
#include <string>
#include <vector>
#include <ranges>

namespace utils {
    class TextParse {
    public:
        TextParse() = default;
        ~TextParse() = default;

        explicit TextParse(const std::string &string) {
            m_data = string_tokenize(string, "\n");
            for (auto &data : m_data) {
                std::replace(data.begin(), data.end(), '\t', '\0');
            }
        };

        static std::vector<std::string> string_tokenize(const std::string &string, const std::string &delimiter = "|") {
            // https://stackoverflow.com/a/48403210
            auto split = string
                | std::ranges::views::split(delimiter)
                | std::ranges::views::transform([](auto &&str) {
                    return std::string_view(&*str.begin(), std::ranges::distance(str));
                });

            // Delete empty strings.
            std::vector<std::string> result{ split.begin(), split.end() };
            result.erase(std::remove_if(result.begin(), result.end(), [](const auto &str) {
                return str.empty();
            }), result.end());

            return result;
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

        std::string get_all_raw() {
            std::string string{};
            for (int i = 0; i < m_data.size(); i++) {
                string += m_data.at(i);

                if (i + 1 >= m_data.size()) {
                    continue;
                }

                if (!m_data.at(i + 1).empty()) {
                    string += '\n';
                }
            }

            return string;
        }

        size_t get_line_count() {
            return m_data.size();
        }

    private:
        std::vector<std::string> m_data;
    };
}
