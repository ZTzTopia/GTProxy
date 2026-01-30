#pragma once
#include <string_view>

namespace sol {
class state;
}

namespace scripting {
class IBindingModule {
public:
    virtual ~IBindingModule() = default;

    [[nodiscard]] virtual std::string_view name() const = 0;
    virtual void bind(sol::state& lua) = 0;
};
}
