#pragma once
#include <functional>

namespace command {
    class Command {
    public:
        Command(const std::string &name, const std::string &description, const std::function<void(const std::vector<std::string> &)> &callback);

        void call(const std::vector<std::string> &args);

        [[nodiscard]] std::string get_name() const { return name; }
        [[nodiscard]] std::string get_description() const { return description; }

    private:
        std::string name;
        std::string description;
        std::function<void(const std::vector<std::string> &)> callback;
    };
}