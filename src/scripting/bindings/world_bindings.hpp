#pragma once
#include <sol/sol.hpp>

#include "../binding_module.hpp"
#include "../../world/world.hpp"

namespace scripting::bindings {
class WorldBindings final : public IBindingModule {
public:
    [[nodiscard]] std::string_view name() const override { return "world"; }

    void bind(sol::state& lua) override;
};
}
