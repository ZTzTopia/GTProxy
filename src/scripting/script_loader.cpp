#include "script_loader.hpp"

#include <algorithm>
#include <spdlog/spdlog.h>

#include "script_engine.hpp"

namespace scripting {

ScriptLoader::ScriptLoader(
    IScriptEngine& engine,
    std::filesystem::path scripts_directory
)
    : engine_{ engine }
    , scripts_directory_{ std::move(scripts_directory) }
{
    if (!std::filesystem::exists(scripts_directory_)) {
        std::filesystem::create_directories(scripts_directory_);
        spdlog::info("Created scripts directory: {}", scripts_directory_.string());
    }
}

std::vector<std::filesystem::path> ScriptLoader::discover_scripts() const
{
    std::vector<std::filesystem::path> scripts{};
    if (!std::filesystem::exists(scripts_directory_)) {
        return scripts;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator{ scripts_directory_ }) {
        if (entry.is_regular_file() && entry.path().extension() == ".lua") {
            scripts.push_back(entry.path());
        }
    }

    std::ranges::sort(scripts, [](const auto& a, const auto& b) {
        return a.filename() < b.filename();
    });

    return scripts;
}

std::size_t ScriptLoader::load_all()
{
    const auto scripts{ discover_scripts() };
    std::size_t loaded_count{ 0 };

    spdlog::info("Discovered {} script(s) in '{}'", scripts.size(), scripts_directory_.string());

    for (const auto& script_path : scripts) {
        if (engine_.execute_file(script_path)) {
            loaded_scripts_.push_back(script_path);
            ++loaded_count;
            spdlog::info("Loaded script: {}", script_path.filename().string());
        }
    }

    spdlog::info("Successfully loaded {}/{} scripts", loaded_count, scripts.size());
    return loaded_count;
}

std::size_t ScriptLoader::reload_all()
{
    spdlog::info("Reloading all scripts...");
    loaded_scripts_.clear();
    return load_all();
}

bool ScriptLoader::load_script(const std::string& filename)
{
    auto script_path{ scripts_directory_ / filename };
    
    if (!script_path.has_extension()) {
        script_path.replace_extension(".lua");
    }

    if (!std::filesystem::exists(script_path)) {
        spdlog::error("Script not found: {}", script_path.string());
        return false;
    }

    if (engine_.execute_file(script_path)) {
        if (
            const auto it{ std::ranges::find(loaded_scripts_, script_path) };
            it == loaded_scripts_.end()
        ) {
            loaded_scripts_.push_back(script_path);
        }

        return true;
    }

    return false;
}
}
