#pragma once
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <charconv>

class TextParse {
public:
    TextParse() = default;

    explicit TextParse(const std::string& str, const std::string& delimiter = "|")
    {
        parse(str, delimiter);
    }

    void parse(std::string_view str, std::string_view delimiter = "|")
    {
        data_.clear();
        for (const auto& line : tokenize(str, "\n")) {
            std::vector tokens{ tokenize(line, delimiter) };

            if (tokens.size() < 2) {
                continue;
            }

            std::string key{ tokens.front() };
            std::vector<std::string> values;
            values.reserve(tokens.size() - 1);

            for (size_t i = 1; i < tokens.size(); ++i) {
                values.emplace_back(tokens[i]);
            }

            data_.emplace(std::move(key), std::move(values));
        }
    }

    static std::vector<std::string_view> tokenize(std::string_view str, std::string_view delimiter = "|", bool keep_empty = true)
    {
        std::vector<std::string_view> tokens{};
        for (auto&& token : str | std::views::split(delimiter)) {
            std::string_view sv{ token.begin(), token.end() };
            if (sv.empty() && !keep_empty) {
                continue;
            }

            tokens.push_back(sv);
        }
        return tokens;
    }

    [[nodiscard]] std::string get(const std::string& key, const int index = 0) const
    {
        const auto it{ data_.find(key) };
        if (it == data_.end()) {
            return {};
        }

        if (index < 0 || index >= static_cast<int>(it->second.size())) {
            return {};
        }

        return it->second[index];
    }

    template <typename T>
    [[nodiscard]] T get(const std::string& key, const int index = 0) const
    {
        std::string val = get(key, index);
        if (val.empty()) return {};

        T result{};
        auto [ptr, ec] = std::from_chars(val.data(), val.data() + val.size(), result);
        if (ec == std::errc{})
 {
            return result;
        }

        // Fallback for types not supported by from_chars (like float/double in some compilers)
        // or if from_chars fails for other reasons.
        try {
            if constexpr (std::is_floating_point_v<T>) {
                if constexpr (std::is_same_v<T, float>) return std::stof(val);
                if constexpr (std::is_same_v<T, double>) return std::stod(val);
                if constexpr (std::is_same_v<T, long double>) return std::stold(val);
            }
        } catch (...) { }

        return {};
    }

    void add(const std::string& key, const std::vector<std::string>& value)
    {
        data_.emplace(key, value);
    }

    void set(const std::string& key, const std::vector<std::string>& value)
    {
        data_[key] = value;
    }

    void remove(const std::string& key)
    {
        data_.erase(key);
    }

    [[nodiscard]] std::string get_raw(const std::string& delimiter = "|", const std::string& prepend_text = "") const
    {
        std::string raw_data{};
        for (auto it = data_.cbegin(); it != data_.cend(); ++it) {
            raw_data += prepend_text + it->first;
            for (const auto& token : it->second) {
                raw_data += delimiter + token;
            }

            if (std::next(it) != data_.cend() && !std::next(it)->first.empty()) {
                raw_data += "\n";
            }
        }

        return raw_data;
    }

    [[nodiscard]] std::vector<std::pair<std::string, std::string>> get_key_values(const std::string& delimiter = "|") const
    {
        std::vector<std::pair<std::string, std::string>> key_values{};
        for (auto it = data_.cbegin(); it != data_.cend(); ++it) {
            std::string value{};
            for (size_t i = 0; i < it->second.size(); ++i) {
                if (i > 0) value += delimiter;
                value += it->second[i];
            }

            key_values.emplace_back(it->first, value);
        }

        return key_values;
    }

    [[nodiscard]] const std::unordered_map<std::string, std::vector<std::string>>& get_data() const { return data_; }
    [[nodiscard]] bool empty() const { return data_.empty(); }

private:
    std::unordered_map<std::string, std::vector<std::string>> data_;
};