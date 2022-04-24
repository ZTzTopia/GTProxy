#pragma once
#include <functional>

namespace command {
    class Command {
    public:
        Command(std::string name, std::string description, std::function<void(const std::vector<std::string> &)> callback);
        ~Command() = default;

        void call(const std::vector<std::string> &args);

        [[nodiscard]] std::string get_name() const { return m_name; }
        [[nodiscard]] std::string get_description() const { return m_description; }

    private:
        std::string m_name;
        std::string m_description;
        std::function<void(const std::vector<std::string> &)> m_callback;
    };
}