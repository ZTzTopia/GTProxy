#pragma once
#include "filesystem"
#include <memory>
#include <string_view>

namespace scripting {
class IBindingModule;

class IScriptEngine {
public:
    virtual ~IScriptEngine() = default;

    virtual bool execute(std::string_view script) = 0;
    virtual bool execute_file(const std::filesystem::path& path) = 0;

    virtual void register_binding(std::unique_ptr<IBindingModule> binding) = 0;

    [[nodiscard]] virtual bool is_valid() const = 0;
};
}
