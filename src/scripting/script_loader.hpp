#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace scripting {
class IScriptEngine;

class ScriptLoader {
public:
    explicit ScriptLoader(
        IScriptEngine& engine,
        std::filesystem::path scripts_directory = "scripts"
    );

    std::size_t load_all();
    std::size_t reload_all();

    bool load_script(const std::string& filename);

    [[nodiscard]] std::vector<std::filesystem::path> discover_scripts() const;

public:
    [[nodiscard]] const std::filesystem::path& scripts_directory() const { return scripts_directory_; }

private:
    IScriptEngine& engine_;
    std::filesystem::path scripts_directory_;
    std::vector<std::filesystem::path> loaded_scripts_;
};
}
