#include "lua_engine.hpp"

#include <spdlog/spdlog.h>

namespace scripting {
LuaEngine::LuaEngine()
{
    open_safe_libraries();
    setup_error_handler();
    spdlog::info("Lua engine initialized successfully");
}

void LuaEngine::open_safe_libraries()
{
    lua_.open_libraries(
        sol::lib::base,
        sol::lib::table,
        sol::lib::string,
        sol::lib::math,
        sol::lib::utf8,
        sol::lib::coroutine
    );
}

void LuaEngine::setup_error_handler()
{
    lua_.set_exception_handler([](lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description) {
        if (maybe_exception) {
            spdlog::error("Lua exception: {}", maybe_exception->what());
        }
        else {
            spdlog::error("Lua error: {}", std::string{ description.data(), description.size() });
        }

        return sol::stack::push(L, description);
    });
}

bool LuaEngine::execute(const std::string_view script)
{
    const sol::protected_function_result result{ lua_.safe_script(
        script,
        sol::script_pass_on_error
    ) };

    if (!result.valid()) {
        const sol::error err = result;
        spdlog::error("Failed to execute script: {}", err.what());
        return false;
    }

    return true;
}

bool LuaEngine::execute_file(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path)) {
        spdlog::error("Script file not found: {}", path.string());
        return false;
    }

    const sol::protected_function_result result{ lua_.safe_script_file(
        path.string(),
        sol::script_pass_on_error
    ) };

    if (!result.valid()) {
        const sol::error err = result;
        spdlog::error("Failed to execute script file '{}': {}", path.string(), err.what());
        return false;
    }

    spdlog::debug("Successfully executed script: {}", path.filename().string());
    return true;
}

void LuaEngine::register_binding(std::unique_ptr<IBindingModule> binding)
{
    if (!binding) {
        spdlog::warn("Attempted to register null binding module");
        return;
    }

    spdlog::debug("Registering binding module: {}", binding->name());
    binding->bind(lua_);
    bindings_.push_back(std::move(binding));
}
}
