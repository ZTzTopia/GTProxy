#pragma once
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <spdlog/spdlog.h>

#include "../binding_module.hpp"

namespace scripting::bindings {
class LoggerBindings final : public IBindingModule {
public:
    [[nodiscard]] std::string_view name() const override { return "logger"; }

    void bind(sol::state& lua) override
    {
        auto logger_table{ lua.create_table() };

        logger_table.set_function("info", [](const std::string& msg) {
            spdlog::info("[Lua] {}", msg);
        });

        logger_table.set_function("warn", [](const std::string& msg) {
            spdlog::warn("[Lua] {}", msg);
        });

        logger_table.set_function("error", [](const std::string& msg) {
            spdlog::error("[Lua] {}", msg);
        });

        logger_table.set_function("debug", [](const std::string& msg) {
            spdlog::debug("[Lua] {}", msg);
        });

        logger_table.set_function("trace", [](const std::string& msg) {
            spdlog::trace("[Lua] {}", msg);
        });

        lua["logger"] = logger_table;
    }
};
}
