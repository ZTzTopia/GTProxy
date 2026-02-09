#pragma once
#include "../binding_module.hpp"

namespace scripting::bindings {
class WorldDataBindings final : public IBindingModule {
public:
    [[nodiscard]] std::string_view name() const override { return "world_data"; }
    void bind(sol::state& lua) override;

 private:
    void bind_tile(sol::state& lua);
    void bind_tile_extra(sol::state& lua);
    void bind_tile_extra_variants(sol::state& lua);
    void bind_object(sol::state& lua);
    void bind_tile_map(sol::state& lua);
    void bind_object_map(sol::state& lua);
};
}
