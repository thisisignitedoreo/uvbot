#include <cstring>
#include <chrono>

#include <Geode/Geode.hpp>

#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/CheckpointObject.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GameObject.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/CCEGLView.hpp>
#include <Geode/modify/CCDrawNode.hpp>

#include "common.hh"

static bool debug_update = false;
static bool override_rotation = false;
static bool level_ended = false;
static std::chrono::steady_clock::time_point level_end_point;

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
        if (uv::bot::recorder::recording) {
            uv::bot::recorder::update();
            if (level_ended) {
                std::chrono::steady_clock::duration excess_amount(std::chrono::milliseconds(static_cast<long long>(uv::bot::recorder::recording_options.excess_render * 1000)));
                if (std::chrono::steady_clock::now() - level_end_point >= excess_amount) {
                    level_ended = false;
                    uv::bot::recorder::end();
                }
            }
        }
        GJBaseGameLayer::update(dt);
        uv::bot::update_physics(this, dt);
    }

#define POLYGON_FROM_RECT(r) { cocos2d::CCPoint((r).getMinX(), (r).getMinY()), cocos2d::CCPoint((r).getMaxX(), (r).getMinY()), cocos2d::CCPoint((r).getMaxX(), (r).getMaxY()), cocos2d::CCPoint((r).getMinX(), (r).getMaxY()) }
    
    void updateDebugDraw(void) {
        debug_update = true;
        this->updateDebugDraw();
        debug_update = false;

        // TODO: custom hitbox rendering

        // Thanks toby
        
        const cocos2d::ccColor4F blank {0.0f, 0.0f, 0.0f, 0.0f};
        const cocos2d::ccColor4F yellow {1.0f, 1.0f, 0.0f, 1.0f};
        const cocos2d::ccColor4F red {1.0f, 0.0f, 0.0f, 1.0f};
        
        cocos2d::CCPoint player_1_polygon[] = POLYGON_FROM_RECT(this->m_player1->getObjectRect());
        this->m_debugDrawNode->drawPolygon(player_1_polygon, 4, blank, 0, yellow);

        if (this->m_gameState.m_isDualMode) {
            cocos2d::CCPoint player_2_polygon[] = POLYGON_FROM_RECT(this->m_player2->getObjectRect());
            this->m_debugDrawNode->drawPolygon(player_2_polygon, 4, blank, 0, yellow);
        }

        if (this->m_anticheatSpike) {
            cocos2d::CCPoint anticheat_spike[] = POLYGON_FROM_RECT(this->m_anticheatSpike->getObjectRect());
            this->m_debugDrawNode->drawPolygon(anticheat_spike, 4, blank, 0, red);
        }
    }
};

class $modify(cocos2d::CCScheduler) {
    void update(float dt) {
        if (uv::bot::recorder::recording) {
            CCScheduler::update(1.0f / uv::bot::recorder::recording_options.fps);
        } else {
            CCScheduler::update(dt);
        }
    }
};

class $modify(HookedCheckpointObject, CheckpointObject) {
    struct Fields {
        uv::practice_fix::checkpoint_data p1, p2;
    };

    bool init() {
        if (CheckpointObject::init()) return false;
        
        PlayLayer *pl = PlayLayer::get();

        if (pl) {
            m_fields->p1 = uv::practice_fix::from_playerobject(pl->m_player1);
            m_fields->p2 = uv::practice_fix::from_playerobject(pl->m_player2);
        }
        
        return true;
    }
};

// Funky GD physics lmao
// Thanks ChatGPT

class $modify(PlayerObject) {
    void updateRotation(float p0) {
        float old_rotation = this->getRotation();
        
        PlayerObject::updateRotation(p0);
        
        if (override_rotation) {
            override_rotation = false;
            this->setRotation(old_rotation);
        }
    }
};

class $modify(EndLevelLayer) {
    void showLayer(bool p0) {
        EndLevelLayer::showLayer(p0);

        if (uv::bot::recorder::recording) {
            this->setVisible(!uv::bot::recorder::recording_options.hide_end_level_screen);
            
            level_ended = true;
            level_end_point = std::chrono::steady_clock::now();
        }
        if (!this->isVisible()) this->setVisible(true);
    }
};

class $modify(PlayLayer) {
    void resetLevel(void) {
        PlayLayer::resetLevel();
        uv::bot::reset();
    }

    void loadFromCheckpoint(CheckpointObject *cp) {
        PlayLayer::loadFromCheckpoint(cp);
        
        if (uv::hacks::practice_fix) {
            uv::practice_fix::restore_playerobject(this->m_player1, static_cast<HookedCheckpointObject*>(cp)->m_fields->p1);
            uv::practice_fix::restore_playerobject(this->m_player2, static_cast<HookedCheckpointObject*>(cp)->m_fields->p2);
            override_rotation = true;
        }
    }
    
    void playEndAnimationToPos(cocos2d::CCPoint p1) {
        PlayLayer::playEndAnimationToPos(p1);
        uv::bot::current_state = uv::bot::state::none;
    }

    void destroyPlayer(PlayerObject *po, GameObject *go) {
        bool let_him_live = uv::hacks::noclip || (uv::hacks::noclip_p1 && po == this->m_player1) || (uv::hacks::noclip_p2 && po == this->m_player2);
        if (!let_him_live) PlayLayer::destroyPlayer(po, go);
    }

    void updateProgressbar(void) {
        PlayLayer::updateProgressbar();
        
        // 0166 -> Is "Show Hitboxes" option on?
        if (uv::hacks::hitboxes && !this->m_isPracticeMode || !GameManager::get()->getGameVariable("0166")) PlayLayer::updateDebugDraw();
        this->m_debugDrawNode->setVisible(uv::hacks::hitboxes);
    }

    void addObject(GameObject *obj) {
        if (uv::hacks::layout_mode) {
            obj->m_activeMainColorID = -1;
            obj->m_activeDetailColorID = -1;
            obj->m_detailUsesHSV = false;
            obj->m_baseUsesHSV = false;
            obj->setOpacity(255);
            obj->setVisible(true);
            
            switch (obj->m_objectID) {
            case 1007: // Alpha
            case 1520: // Shake
                return;
            }
        }
        
        PlayLayer::addObject(obj);
    }
};

class $modify(GameObject) {
    void addGlow(gd::string p0) {
        GameObject::addGlow(p0);

        if (uv::hacks::layout_mode && this->m_objectType == GameObjectType::Decoration) this->m_isHide = true;
    }

    // Thanks cvolton, unthanks robtop
    void determineSlopeDirection() {
        if (debug_update) return;
        GameObject::determineSlopeDirection();
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

class $modify(cocos2d::CCScheduler) {
    void update(float dt) {
        if (uv::hacks::speedhack) dt *= uv::hacks::speedhack_multiplier;
        CCScheduler::update(dt);
    }
};

class $modify(cocos2d::CCEGLView) {
    void onGLFWKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        CCEGLView::onGLFWKeyCallback(window, key, scancode, action, mods);

        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_TAB) {
                uv::gui::toggle_time = std::chrono::steady_clock::now();
                uv::gui::show = !uv::gui::show;
            }
            if (key == GLFW_KEY_7 && mods == GLFW_MOD_CONTROL && uv::gui::show) uv::gui::debug = !uv::gui::debug;
        }
    }
};

class $modify(cocos2d::CCDrawNode) {
    bool drawPolygon(cocos2d::CCPoint *vertex, unsigned int count, const cocos2d::ccColor4F &fillColor, float thickness, const cocos2d::ccColor4F &borderColor) {
        thickness = uv::hacks::hitboxes_thickness;

        return cocos2d::CCDrawNode::drawPolygon(vertex, count, fillColor, thickness, borderColor);
    }
    
    bool drawCircle(const cocos2d::CCPoint &position, float radius, const cocos2d::ccColor4F &color, float thickness, const cocos2d::ccColor4F &borderColor, unsigned int segments) {
        thickness = uv::hacks::hitboxes_thickness;
        this->m_bUseArea = false;

        return cocos2d::CCDrawNode::drawCircle(position, radius, color, thickness, borderColor, segments);
    }
};
