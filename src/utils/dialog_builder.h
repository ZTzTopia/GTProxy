#pragma once
#include <string>
#include <string_view>
#include "./fmt/format.h"

class dialog_builder
{
public:
    enum size_type {
        SMALL,
        BIG
    };
    enum direction {
        NONE,
        LEFT,
        RIGHT,
        STATIC_BLUE_FRAME
    };
private:
    std::string get_direction(const direction& dir) {
        switch (dir) {
        case direction::LEFT:
            return "left";
        case direction::RIGHT:
            return "right";
        case direction::STATIC_BLUE_FRAME:
            return "staticBlueFrame";
        default:
            return "";
        }
        return "";
    }

public:
    dialog_builder() : m_result("") {}
    ~dialog_builder() {}

    std::string get() {
        return m_result;
    }
    void clear() {
        m_result.clear();
    }
    
    dialog_builder* set_default_color(const char& color) {
        m_result.append(fmt::format("\nset_default_color|`{}", color));
        return this;
    }
    dialog_builder* text_scaling_string(const std::string& scale) {
        m_result.append(fmt::format("\ntext_scaling_string|{}", scale));
        return this;
    }

    dialog_builder* embed_data(const std::string& name, const std::string& embed_value) {
        m_result.append(fmt::format("\nembed_data|{}|{}", name, embed_value));
        return this;
    }
    template<typename T, typename std::enable_if_t<std::is_integral_v<T>, bool> = true>
    dialog_builder* embed_data(const std::string& name, const T& embed_value) {
        m_result.append(fmt::format("\nembed_data|{}|{}", name, (int)embed_value));
        return this;
    }
    dialog_builder* end_dialog(const std::string& name, const std::string& cancel, const std::string& ok) {
        m_result.append(fmt::format("\nend_dialog|{}|{}|{}|", name, cancel, ok));
        return this;
    }

    dialog_builder* add_spacer(const size_type& label_size = size_type::SMALL) {
        m_result.append(fmt::format("\nadd_spacer|{}|", label_size == size_type::SMALL ? "small" : "big"));
        return this;
    }
    dialog_builder* add_label_with_icon(const std::string& label, const int& itemId, const direction& dir = LEFT, const size_type& label_size = SMALL) {
        m_result.append(fmt::format("\nadd_label_with_icon|{}|{}|{}|{}|", label_size == size_type::SMALL ? "small" : "big", label, this->get_direction(dir), itemId));
        return this;
    }
    dialog_builder* add_textbox(const std::string& text) {
        m_result.append(fmt::format("\nadd_textbox|{}|", text));
        return this;
    }
    dialog_builder* add_smalltext(const std::string& text) {
        m_result.append(fmt::format("\nadd_smalltext|{}|", text));
        return this;
    }
    dialog_builder* add_quick_exit() {
        m_result.append("\nadd_quick_exit");
        return this;
    }
    dialog_builder* add_button(const std::string& name, const std::string& text) {
        m_result.append(fmt::format("\nadd_button|{}|{}|noflags|0|0", name, text));
        return this;
    }
private:
    std::string m_result;
};