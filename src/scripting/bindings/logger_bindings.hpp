#pragma once
#include <sol/sol.hpp>
#include <spdlog/spdlog.h>

#include "../binding_module.hpp"

namespace scripting::bindings {
class LoggerBindings final : public IBindingModule {
public:
    [[nodiscard]] std::string_view name() const override { return "logger"; }

    void bind(sol::state& lua) override;
};
}
