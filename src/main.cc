#include <cstring>
#include <chrono>

#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/CheckpointObject.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/CCEGLView.hpp>

#include <imgui-cocos.hpp>

#include "common.hh"

using namespace geode::prelude;

class $modify(GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        if (uv::bot::button(down, button, isPlayer1)) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
        }
    }
    
    void processCommands(float dt) {
        GJBaseGameLayer::processCommands(dt);
        uv::bot::update_input(this, dt);
    }
    
    void update(float dt) {
        GJBaseGameLayer::update(dt);
        uv::bot::update_physics(this, dt);
    }
};

class $modify(HookedCheckpointObject, CheckpointObject) {
    struct Fields {
        uv::practice_fix::checkpoint_data p1, p2;
    };

    bool init() {
        bool r = CheckpointObject::init();
        if (!r) return r;

        PlayLayer *pl = PlayLayer::get();

        if (pl) {
            m_fields->p1 = uv::practice_fix::from_playerobject(pl->m_player1);
            m_fields->p2 = uv::practice_fix::from_playerobject(pl->m_player2);
        }
        
        return r;
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

    void loadFromCheckpoint(CheckpointObject* cp) {
        PlayLayer::loadFromCheckpoint(cp);
        
        uv::practice_fix::restore_playerobject(this->m_player1, static_cast<HookedCheckpointObject*>(cp)->m_fields->p1);
        uv::practice_fix::restore_playerobject(this->m_player2, static_cast<HookedCheckpointObject*>(cp)->m_fields->p2);
    }

    void destroyPlayer(PlayerObject *po, GameObject *go) {
        bool let_him_live = uv::hacks::noclip || (uv::hacks::noclip_p1 && po == this->m_player1) || (uv::hacks::noclip_p2 && po == this->m_player2);
        if (!let_him_live) PlayLayer::destroyPlayer(po, go);
    }
};

class $modify(PlayerObject) {
    void resetLevel(void) {
        this->resetLevel();
        uv::bot::reset();
    }

    void playEndAnimationToPos(cocos2d::CCPoint p1) {
	this->playEndAnimationToPos(p1);
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
            if (key == GLFW_KEY_MINUS && uv::gui::show) uv::gui::debug = true;
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
