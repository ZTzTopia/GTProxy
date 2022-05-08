#pragma once
#include <functional>
#include <string>
#include <utility>

namespace command {
    struct CommandContext {
        std::string name;
        std::vector<std::string> aliases;
        std::string description;
    };

    class Command {
    public:
        Command(CommandContext context,
            const std::function<void(const std::vector<std::string>& args)>& callback)
            : m_context(std::move(context)), m_callback(callback) {}
        ~Command() = default;

        void call(const std::vector<std::string>& args) { m_callback(args); }

        [[nodiscard]] CommandContext get_context() const { return m_context; }
        [[nodiscard]] std::string get_name() const { return m_context.name; }
        [[nodiscard]] std::vector<std::string> get_aliases() const { return m_context.aliases; }
        [[nodiscard]] std::string get_description() const { return m_context.description; }

    private:
        CommandContext m_context;
        std::function<void(std::vector<std::string>)> m_callback;
    };
}// namespace command