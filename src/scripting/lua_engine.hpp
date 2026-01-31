#pragma once
#include <memory>
#include <vector>

#include <sol/sol.hpp>

#include "binding_module.hpp"
#include "script_engine.hpp"
#include "../utils/types.hpp"

namespace scripting {
class LuaEngine final : public IScriptEngine, public utils::types::Immobile {
public:
    LuaEngine();
    ~LuaEngine() override = default;

    bool execute(std::string_view script) override;
    bool execute_file(const std::filesystem::path& path) override;
    void register_binding(std::unique_ptr<IBindingModule> binding) override;

public:
    [[nodiscard]] bool is_valid() const override { return lua_.lua_state() != nullptr; }

    [[nodiscard]] sol::state& state() { return lua_; }
    [[nodiscard]] const sol::state& state() const { return lua_; }

private:
    void open_safe_libraries();
    void setup_error_handler();

private:
    sol::state lua_;
    std::vector<std::unique_ptr<IBindingModule>> bindings_;
};
}
