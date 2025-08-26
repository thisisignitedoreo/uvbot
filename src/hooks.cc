#include <sstream>
#include <cstring>
#include <chrono>

#include <Geode/Geode.hpp>

#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/CheckpointObject.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/UILayer.hpp>
#include <Geode/modify/GameObject.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/CCEGLView.hpp>
#include <Geode/modify/CCDrawNode.hpp>

#include "common.hh"

static bool debug_update = false;
static bool override_rotation = false;
static bool level_ended = false;
static bool started_audio_render = false;
static std::chrono::steady_clock::time_point level_end_point;
static EndLevelLayer *ell;
static std::chrono::steady_clock::time_point last_time, now;
static std::chrono::steady_clock::duration accumulator;

class $modify(HookedEndLevelLayer, EndLevelLayer) {
    struct Fields {
        cocos2d::CCSprite *black_fg;
    };

    void update_fade_out(float) {
        if (m_fields->black_fg) {
            if (uv::recorder::recording) {
                std::chrono::steady_clock::duration excess(std::chrono::milliseconds(static_cast<long long>(uv::recorder::recording_options.excess_render * 1000)));
                float opacity = (std::chrono::steady_clock::now() - level_end_point).count() / static_cast<float>(excess.count());
                m_fields->black_fg->setOpacity(static_cast<unsigned char>(opacity * 255.0f));
            } else m_fields->black_fg->setOpacity(0);
        }
        if (!uv::recorder::recording) this->setVisible(true);
    }
    
    void showLayer(bool p0) {
        EndLevelLayer::showLayer(p0);

        if (uv::recorder::recording || uv::recorder::audio::recording) {
            level_ended = true;
            level_end_point = std::chrono::steady_clock::now();
            ell = this;

            PlayLayer *pl = PlayLayer::get();
            if (uv::recorder::recording && pl) {
                cocos2d::CCSize wnd_size = cocos2d::CCDirector::sharedDirector()->getWinSize();
                
                m_fields->black_fg = cocos2d::CCSprite::create("game_bg_13_001.png");
                
                cocos2d::CCSize spr_size = m_fields->black_fg->getContentSize();
                m_fields->black_fg->setPosition({ wnd_size.width / 2, wnd_size.height / 2 });            
                m_fields->black_fg->setScaleX(wnd_size.width / spr_size.width * 2.f);
                m_fields->black_fg->setScaleY(wnd_size.height / spr_size.height * 2.f);
                m_fields->black_fg->setColor({ 0, 0, 0 });
                m_fields->black_fg->setOpacity(0);
                m_fields->black_fg->setZOrder(1000);

                pl->addChild(m_fields->black_fg);
            }
        }
        
        this->schedule(schedule_selector(HookedEndLevelLayer::update_fade_out), 0.0f);
        this->setVisible(!uv::recorder::recording || !uv::recorder::recording_options.hide_end_level_screen);
    }
};

class $modify(GJBaseGameLayer) {
    struct Fields {
        float initial_x = -0.0f;
        bool ready_to_render = false;
    };

    /*bool init(void) {
        if (!GJBaseGameLayer::init()) return false;

        m_fields->initial_x = -0.0f;
        
        return true;
    }*/
    
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
        // Weird hack to bypass start level lil delay
        if (m_fields->initial_x == -0.0f) m_fields->initial_x = this->m_player1->m_position.x;
        m_fields->ready_to_render = m_fields->initial_x < this->m_player1->m_position.x;
        
        if (!uv::recorder::recording && uv::recorder::audio::recording && !started_audio_render && m_fields->ready_to_render) {
            started_audio_render = true;
            uv::recorder::audio::start();
        }
        
        if (uv::recorder::recording && m_fields->ready_to_render) {
            PlayLayer *pl = PlayLayer::get();
            if (pl) pl->processActivatedAudioTriggers(pl->m_gameState.m_levelTime);
            uv::recorder::update();
            if (level_ended) {
                std::chrono::steady_clock::duration excess_amount(std::chrono::milliseconds(static_cast<long long>(uv::recorder::recording_options.excess_render * 1000)));
                if (std::chrono::steady_clock::now() - level_end_point >= excess_amount) {
                    level_ended = false;
                    uv::recorder::end();
                }
            }
        }
        
        if (uv::recorder::audio::recording) {
            if (level_ended) {
                std::chrono::steady_clock::duration excess_amount(std::chrono::milliseconds(static_cast<long long>(uv::recorder::audio::recording_options.excess_render * 1000)));
                if (std::chrono::steady_clock::now() - level_end_point >= excess_amount) {
                    level_ended = false;
                    started_audio_render = false;
                    uv::recorder::audio::end();
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

    // Thanks toby
    void updateColor(cocos2d::ccColor3B &color, float fadeTime, int colorID, bool blending, float opacity, cocos2d::ccHSVValue &copyHSV, int colorIDToCopy, bool copyOpacity, EffectGameObject *callerObject, int unk1, int unk2) {
        PlayLayer *pl = PlayLayer::get();
        if (!pl) return GJBaseGameLayer::updateColor(color, fadeTime, colorID, blending, opacity, copyHSV, colorIDToCopy, copyOpacity, callerObject, unk1, unk2);

        cocos2d::ccColor3B copy_color = color;
        if (uv::hacks::layout_mode) {
            if (colorID == 1000) copy_color = {0, 0, 0};
            else if (colorID == 1001) copy_color = {0, 0, 0};
            else if (colorID == 1002) copy_color = {255, 255, 255};
            else if (colorID == 1009) copy_color = {0, 0, 0};
            else if (colorID == 1013 || colorID == 1014) copy_color = {0, 0, 0};
            else copy_color = {255, 255, 255};
        }

        GJBaseGameLayer::updateColor(copy_color, fadeTime, colorID, blending, opacity, copyHSV, colorIDToCopy, copyOpacity, callerObject, unk1, unk2);
    }
};

class $modify(cocos2d::CCDirector) {
    bool init(void) {
        if (!cocos2d::CCDirector::init()) return false;
        last_time = std::chrono::steady_clock::now();
        return true;
    }
    
    void drawScene(void) {
        if (uv::hacks::speedhack && uv::hacks::speedhack_classic) {
            now = std::chrono::steady_clock::now();
            accumulator += now - last_time;
            last_time = now;

            double frame_length = this->getAnimationInterval() * (1.0 / uv::hacks::speedhack_multiplier);
            
            if (std::chrono::duration<double>(accumulator).count() < frame_length) return;
            accumulator -= std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(accumulator));
        }

        cocos2d::CCDirector::drawScene();
    }
};

class $modify(cocos2d::CCScheduler) {
    void update(float dt) {
        if (uv::recorder::recording) {
            cocos2d::CCScheduler::update(1.0f / uv::recorder::recording_options.fps);
        } else {
            cocos2d::CCScheduler::update(uv::hacks::lock_delta ? cocos2d::CCDirector::get()->getAnimationInterval() : dt);
        }
    }
};

class $modify(HookedCheckpointObject, CheckpointObject) {
    struct Fields {
        uv::practice_fix::checkpoint_data p1, p2;
    };

    bool init(void) {
        //  | This one byte of code just cost me an hour of painful debugging
        //  v I will never get that time back
        if (!CheckpointObject::init()) return false;
        
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
        if (uv::bot::current_state == uv::bot::state::recording) uv::bot::current_state = uv::bot::state::none;
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
            obj->m_detailUsesHSV = false;
            obj->m_baseUsesHSV = false;
            obj->setOpacity(255);
            obj->setVisible(true);
            
            switch (obj->m_objectID) {
            // Thanks toby
            case 899:
            case 900:
            case 29:
            case 30:
            case 104:
            case 105:
            case 221:
            case 717:
            case 718:
            case 743:
            case 744:
            case 915:
            case 1006:
            case 1007:
            case 1520:
            case 2903:
            case 3029:
            case 3030:
            case 3031:
            case 2999:
            case 3606:
            case 3612:
            case 3010:
            case 3015:
            case 3021:
            case 3009:
            case 3014:
            case 3020:
            case 1818:
            case 1819:
            case 33:
            case 32:
            case 1613:
            case 1612:
            case 3608:
                return;
            }
        }
        
        PlayLayer::addObject(obj);
    }
};

// Thanks toby
class $modify(UILayer) {
    bool init(GJBaseGameLayer *p0) {
        if (!UILayer::init(p0)) return false;
        
        if (!uv::recorder::recording) return true;
        
        cocos2d::CCMenu *menu = getChildByType<cocos2d::CCMenu>(0);
        CCMenuItemSpriteExtra *btn = menu->getChildByType<CCMenuItemSpriteExtra>(0);

        if (menu && btn) btn->getNormalImage()->setVisible(false);

        return true;
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

class $modify(cocos2d::CCScheduler) {
    void update(float dt) {
        if (uv::hacks::speedhack && !uv::hacks::speedhack_classic) dt *= uv::hacks::speedhack_multiplier;
        CCScheduler::update(dt);
    }
};

class $modify(cocos2d::CCEGLView) {
    void onGLFWKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        CCEGLView::onGLFWKeyCallback(window, key, scancode, action, mods);

        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_TAB) {
                uv::gui::toggle_time = std::chrono::steady_clock::now();
                if (!(uv::gui::show = !uv::gui::show)) uv::hacks::save();
            }
            if (key == GLFW_KEY_7 && mods == GLFW_MOD_CONTROL && uv::gui::show) uv::gui::debug = !uv::gui::debug;
        }
    }
};

#define COLOR4F_EQUALS(c, cr, cg, cb) ((c).r == (cr) && (c).g == (cg) && (c).b == (cb))
#define FLOAT4_TO_COLOR4F(a) (cocos2d::ccColor4F { (a)[0], (a)[1], (a)[2], (a)[3] })

class $modify(cocos2d::CCDrawNode) {
    bool drawPolygon(cocos2d::CCPoint *vertex, unsigned int count, const cocos2d::ccColor4F &fillColor, float thickness, const cocos2d::ccColor4F &borderColor) {
        thickness = uv::hacks::hitboxes_thickness;

        cocos2d::ccColor4F copy_color, copy_fill_color;
        if (COLOR4F_EQUALS(borderColor, 1.0f, 0.0f, 0.0f)) {
            copy_color = FLOAT4_TO_COLOR4F(uv::hacks::hitboxes_color_hazards);
            copy_fill_color = FLOAT4_TO_COLOR4F(uv::hacks::hitboxes_color_fill_hazards);
        } else if (COLOR4F_EQUALS(borderColor, 0.0f, 0.25f, 1.0f)) {
            copy_color = FLOAT4_TO_COLOR4F(uv::hacks::hitboxes_color_solids);
            copy_fill_color = FLOAT4_TO_COLOR4F(uv::hacks::hitboxes_color_fill_solids);
        } else if (COLOR4F_EQUALS(borderColor, 0.0f, 1.0f, 0.0f)) {
            copy_color = FLOAT4_TO_COLOR4F(uv::hacks::hitboxes_color_specials);
            copy_fill_color = FLOAT4_TO_COLOR4F(uv::hacks::hitboxes_color_fill_specials);
        } else if (COLOR4F_EQUALS(borderColor, 1.0f, 1.0f, 0.0f)) {
            copy_color = FLOAT4_TO_COLOR4F(uv::hacks::hitboxes_color_player);
            copy_fill_color = FLOAT4_TO_COLOR4F(uv::hacks::hitboxes_color_fill_player);
        } else {
            copy_color = borderColor;
            copy_fill_color = fillColor;
        }

        return cocos2d::CCDrawNode::drawPolygon(vertex, count, copy_fill_color, thickness, copy_color);
    }
    
    bool drawCircle(const cocos2d::CCPoint &position, float radius, const cocos2d::ccColor4F &color, float thickness, const cocos2d::ccColor4F &borderColor, unsigned int segments) {
        thickness = uv::hacks::hitboxes_thickness;
        this->m_bUseArea = false;

        return cocos2d::CCDrawNode::drawCircle(position, radius, color, thickness, borderColor, segments);
    }
};
