#include <Geode/Geode.hpp>

#include <imgui-cocos.hpp>

#include "common.hh"

$on_mod(Loaded) {
    ImGuiCocos::get().setup(uv::gui::setup).draw(uv::gui::draw);
    
    std::vector<std::filesystem::path> paths = {
        geode::Mod::get()->getSaveDir() / "Macros",
        geode::Mod::get()->getSaveDir() / "Showcases",
    };

    std::error_code error;

    for (auto &path : paths) {
        if (!std::filesystem::exists(path, error) || !std::filesystem::is_directory(path, error)) {
            if (!geode::utils::file::createDirectory(path)) {
                geode::log::warn("Couldn't create '{}'", path);
            }
        }
    }
}
