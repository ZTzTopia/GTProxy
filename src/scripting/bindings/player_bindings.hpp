#pragma once
#include <sol/sol.hpp>

#include "../binding_module.hpp"
#include "../../player/player.hpp"

namespace scripting::bindings {
class PlayerBindings final : public IBindingModule {
public:
    [[nodiscard]] std::string_view name() const override { return "player"; }

    void bind(sol::state& lua) override;
};
}
