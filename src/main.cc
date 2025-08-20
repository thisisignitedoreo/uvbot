#include <Geode/Geode.hpp>

#include <imgui-cocos.hpp>

#include "common.hh"

$on_mod(Loaded) {
    ImGuiCocos::get().setup(uv::gui::setup).draw(uv::gui::draw);
    
    std::vector<std::filesystem::path> paths = {
        geode::Mod::get()->getSaveDir() / "Macros",
        geode::Mod::get()->getSaveDir() / "Showcases",
    };

    for (auto &path : paths) {
        if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
            std::filesystem::create_directory(path);
        }
    }
}
