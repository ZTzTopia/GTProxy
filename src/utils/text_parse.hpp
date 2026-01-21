#pragma once
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
#include <charconv>
#include <algorithm>
#include <type_traits>
#include <array>
#include <sstream>
#include <utility>

class TextParse {
public:
    TextParse() = default;
    explicit TextParse(const std::string& str, const std::string_view& delimiter = "|")
    {
        parse(str, delimiter);
    }

    void parse(const std::string_view str, const std::string_view delimiter = "|")
    {
        if (!data_.empty()) {
            data_.clear();
        }

        const auto line_count{ std::ranges::count(str, '\n') + 1 };
        data_.reserve(line_count);

        for (const auto& line : tokenize(str, "\n")) {
            auto tokens{ tokenize(line, delimiter) };
            if (tokens.size() < 2) {
                continue;
            }

            std::string key{ tokens.front() };
            std::vector<std::string> values{};
            values.reserve(tokens.size() - 1);

            for (size_t i{ 1 }; i < tokens.size(); ++i) {
                values.emplace_back(tokens[i]);
            }

            data_.emplace_back(std::move(key), std::move(values));
        }
    }

    static std::vector<std::string_view> tokenize(std::string_view str, std::string_view delimiter = "|", const bool keep_empty = true)
    {
        std::vector<std::string_view> tokens{};
        if (str.empty()) {
            return tokens;
        }

        for (auto&& token : str | std::views::split(delimiter)) {
            std::string_view sv{ token.begin(), token.end() };
            if (sv.empty()) {
                if (!keep_empty || tokens.empty()) {
                    continue;
                }
            }

            tokens.push_back(sv);
        }

        return tokens;
    }

    template <typename T = std::string>
    [[nodiscard]] T get(const std::string& key, int index = 0) const
    {
        const auto it{ std::ranges::find_if(data_, [&key](const auto& pair) {
            return pair.first == key;
        }) };

        if (it == data_.end()) {
            return {};
        }

        const auto& vec{ it->second };

        if (index < 0 || index >= static_cast<int>(vec.size())) {
            return {};
        }

        const std::string& val{ vec[index] };

        if constexpr (std::is_same_v<T, std::string>) {
            return val;
        }
        else {
            if (val.empty()) {
                return {};
            }

#ifdef __clang__
            if constexpr (std::is_integral_v<T>) {
#else
            if constexpr (std::is_arithmetic_v<T>) {
#endif
                T result{};
                auto [ptr, ec] = std::from_chars(val.data(), val.data() + val.size(), result);
                if (ec == std::errc{}) {
                    return result;
                }
            }
#if __clang__
            else if constexpr (std::is_floating_point_v<T>) {
                // Use std::from_chars for floats as a reliable fallback for cases where
                // std::from_chars for floating-point is marked as 'deleted' in the library.
                try {
                    if constexpr (std::is_floating_point_v<T>) {
                        if constexpr (std::is_same_v<T, float>) {
                            return std::stof(val);
                        }

                        if constexpr (std::is_same_v<T, double>) {
                            return std::stod(val);
                        }

                        if constexpr (std::is_same_v<T, long double>) {
                            return std::stold(val);
                        }
                    }
                } catch (...) { }
            }
#endif
        }

        return {};
    }

    void add(const std::string& key, const std::vector<std::string>& value)
    {
        data_.emplace_back(key, value);
    }

    void add(std::string&& key, std::vector<std::string>&& value)
    {
        data_.emplace_back(std::move(key), std::move(value));
    }

    template <typename... Args>
    void add(const std::string& key, Args&&... args)
    {
        std::vector<std::string> values{};
        values.reserve(sizeof...(args));
        (values.emplace_back(convert_to_string(std::forward<Args>(args))), ...);
        data_.emplace_back(key, std::move(values));
    }

    void set(const std::string& key, const std::vector<std::string>& value)
    {
        const auto it{ std::ranges::find_if(data_, [&key](const auto& pair) {
            return pair.first == key;
        }) };

        if (it != data_.end()) {
            it->second = value;
            return;
        }

        data_.emplace_back(key, value);
    }

    void set(std::string&& key, std::vector<std::string>&& value)
    {
        const auto it{ std::ranges::find_if(data_, [&key](const auto& pair) {
            return pair.first == key;
        }) };

        if (it != data_.end()) {
            it->second = std::move(value);
            return;
        }

        data_.emplace_back(std::move(key), std::move(value));
    }

    template <typename... Args>
    void set(const std::string& key, Args&&... args)
    {
        auto it{ std::ranges::find_if(data_, [&key](const auto& pair) {
            return pair.first == key;
        }) };

        if (it != data_.end()) {
            std::vector<std::string> values{};
            values.reserve(sizeof...(args));
            (values.emplace_back(convert_to_string(std::forward<Args>(args))), ...);
            it->second = std::move(values);
        }
        else {
            add(key, std::forward<Args>(args)...);
        }
    }

    void remove(const std::string& key)
    {
        const auto it{ std::ranges::find_if(data_, [&key](const auto& pair) {
            return pair.first == key;
        }) };
        if (it != data_.end()) {
            data_.erase(it);
        }
    }

    [[nodiscard]] std::string get_raw(const std::string& delimiter = "|", const std::string& prepend_text = "") const
    {
        size_t estimated_size{ 0 };
        for (const auto& [key, values] : data_) {
            estimated_size += prepend_text.size() + key.size() + 1; // +1 for possible \n
            for (const auto& val : values) {
                estimated_size += delimiter.size() + val.size();
            }
        }

        std::string raw_data{};
        raw_data.reserve(estimated_size);

        for (auto it{ data_.cbegin() }; it != data_.cend(); ++it) {
            raw_data += prepend_text;
            raw_data += it->first;

            for (const auto& token : it->second) {
                raw_data += delimiter;
                raw_data += token;
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
        key_values.reserve(data_.size());

        for (const auto& [key, values] : data_) {
            size_t val_size{ 0 };
            for (size_t i{ 0 }; i < values.size(); ++i) {
                if (i > 0) {
                    val_size += delimiter.size();
                }

                val_size += values[i].size();
            }

            std::string value{};
            value.reserve(val_size);

            for (size_t i{ 0 }; i < values.size(); ++i) {
                if (i > 0) value += delimiter;
                value += values[i];
            }

            key_values.emplace_back(key, std::move(value));
        }

        return key_values;
    }

public:
    [[nodiscard]] const std::vector<std::pair<std::string, std::vector<std::string>>>& get_data() const { return data_; }
    [[nodiscard]] bool empty() const { return data_.empty(); }

    [[nodiscard]] bool contains(const std::string& key) const
    {
        return std::ranges::any_of(data_, [&key](const auto& pair) {
            return pair.first == key;
        });
    }

private:
    template<typename T>
    static std::string convert_to_string(const T& value)
    {
        if constexpr (std::is_same_v<T, std::string>) {
            return value;
        }
        else if constexpr (std::is_same_v<T, std::string_view>) {
            return std::string{ value };
        }
        else if constexpr (std::is_same_v<T, bool>) {
            return value ? "1" : "0";
        }
#if __clang__
        else if constexpr (std::is_integral_v<T>) {
#else
        else if constexpr (std::is_arithmetic_v<T>) {
#endif
            std::array<char, 64> buffer{};
            if (auto result{ std::to_chars(buffer.data(), buffer.data() + buffer.size(), value) };
                result.ec == std::errc{}) {
                return std::string{ buffer.data(), result.ptr };
            }
            return {};
        }
#if __clang__
        else if constexpr (std::is_floating_point_v<T>) {
            // Use std::to_string for floats as a reliable fallback for cases where
            // std::to_chars for floating-point is marked as 'deleted' in the library.
            return std::to_string(value);
        }
#endif
        else {
            std::ostringstream oss;
            oss << value;
            return oss.str();
        }
    }

private:
    std::vector<std::pair<std::string, std::vector<std::string>>> data_;
};
