#pragma once
#include "../binding_module.hpp"

namespace scripting::bindings {
class ItemBindings final : public IBindingModule {
public:
    [[nodiscard]] std::string_view name() const override { return "item"; }
    void bind(sol::state& lua) override;

private:
    void bind_item_database(sol::state& lua);
    void bind_item_info(sol::state& lua);
    void bind_enums(sol::state& lua);
};
}
