#include <Geode/Geode.hpp>

#include <imgui-cocos.hpp>

#include "common.hh"

using namespace geode::prelude;

$on_mod(Loaded) {
    ImGuiCocos::get().setup(uv::gui::setup).draw(uv::gui::draw);
    
    std::filesystem::path path = geode::Mod::get()->getSaveDir() / "Macros";
    
    if (!std::filesystem::is_directory(path) || !std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
    }
}
