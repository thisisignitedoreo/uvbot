#include <cstring>
#include <chrono>

#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/CCEGLView.hpp>

#include <imgui-cocos.hpp>

#include "common.hh"

using namespace geode::prelude;

class $modify(GJBaseGameLayer) {
    void processCommands(float dt) {
        GJBaseGameLayer::processCommands(dt);
        uv::bot::update(this, dt);
    }

    void handleButton(bool down, int button, bool isPlayer1) {
        if (uv::bot::button(down, button, isPlayer1)) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
        }
    }
};

class $modify(PlayLayer) {
    void resetLevel(void) {
        PlayLayer::resetLevel();
        uv::bot::reset();
    }

    void playEndAnimationToPos(cocos2d::CCPoint p1) {
	PlayLayer::playEndAnimationToPos(p1);
	uv::bot::current_state = uv::bot::state::none;
    }
};

class $modify(CCScheduler) {
    void update(float dt) {
        if (uv::hacks::speedhack) dt *= uv::hacks::speedhack_multiplier;
        CCScheduler::update(dt);
    }
};

class $modify(cocos2d::CCEGLView) {
    void onGLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        CCEGLView::onGLFWKeyCallback(window, key, scancode, action, mods);

        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_TAB) {
                uv::gui::toggle_time = std::chrono::steady_clock::now();
                uv::gui::show = !uv::gui::show;
            }
        }
    }
};

$on_mod(Loaded) {
    ImGuiCocos::get().setup(uv::gui::setup).draw(uv::gui::draw);
    
    std::filesystem::path path = geode::Mod::get()->getSaveDir() / "Macros";
    
    if (!std::filesystem::is_directory(path) || !std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
    }
}
