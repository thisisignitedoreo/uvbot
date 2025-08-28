#include <Geode/Geode.hpp>

#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GameObject.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/CCDrawNode.hpp>
#include <Geode/modify/CCScheduler.hpp>

#include "common.hh"

namespace uv::hacks {
    bool speedhack;
    bool speedhack_classic;
    float speedhack_multiplier;
    bool lock_delta;
    bool noclip;
    bool noclip_p1;
    bool noclip_p2;
    bool hitboxes, hitboxes_trajectory;
    int hitboxes_trajectory_length;
    float hitboxes_thickness;
    std::vector<float> hitboxes_color_hazards, hitboxes_color_fill_hazards;
    std::vector<float> hitboxes_color_solids, hitboxes_color_fill_solids;
    std::vector<float> hitboxes_color_specials, hitboxes_color_fill_specials;
    std::vector<float> hitboxes_color_player, hitboxes_color_fill_player;
    bool layout_mode;
    bool practice_fix;
    bool copy_hack;
    
    void save(void) {
        geode::Mod::get()->setSavedValue<bool>("speedhack", speedhack);
        geode::Mod::get()->setSavedValue<bool>("speedhack-classic", speedhack_classic);
        geode::Mod::get()->setSavedValue<float>("speedhack-multiplier", speedhack_multiplier);
        geode::Mod::get()->setSavedValue<bool>("lock-delta", lock_delta);
        geode::Mod::get()->setSavedValue<bool>("noclip", noclip);
        geode::Mod::get()->setSavedValue<bool>("noclip-p1", noclip_p1);
        geode::Mod::get()->setSavedValue<bool>("noclip-p2", noclip_p2);
        geode::Mod::get()->setSavedValue<bool>("hitboxes", hitboxes);
        geode::Mod::get()->setSavedValue<bool>("hitboxes-trajectory", hitboxes_trajectory);
        geode::Mod::get()->setSavedValue<int>("hitboxes-trajectory-length", hitboxes_trajectory_length);
        geode::Mod::get()->setSavedValue<float>("hitboxes-thickness", hitboxes_thickness);
        geode::Mod::get()->setSavedValue<std::vector<float>>("hitboxes-color-hazards", hitboxes_color_hazards);
        geode::Mod::get()->setSavedValue<std::vector<float>>("hitboxes-color-fill-hazards", hitboxes_color_fill_hazards);
        geode::Mod::get()->setSavedValue<std::vector<float>>("hitboxes-color-solids", hitboxes_color_solids);
        geode::Mod::get()->setSavedValue<std::vector<float>>("hitboxes-color-fill-solids", hitboxes_color_fill_solids);
        geode::Mod::get()->setSavedValue<std::vector<float>>("hitboxes-color-specials", hitboxes_color_specials);
        geode::Mod::get()->setSavedValue<std::vector<float>>("hitboxes-color-fill-specials", hitboxes_color_fill_specials);
        geode::Mod::get()->setSavedValue<std::vector<float>>("hitboxes-color-player", hitboxes_color_player);
        geode::Mod::get()->setSavedValue<std::vector<float>>("hitboxes-color-fill-player", hitboxes_color_fill_player);
        geode::Mod::get()->setSavedValue<bool>("layout-mode", layout_mode);
        geode::Mod::get()->setSavedValue<bool>("practice-fix", practice_fix);
        geode::Mod::get()->setSavedValue<bool>("copy-hack", copy_hack);
    }

    void load(void) {
        speedhack = geode::Mod::get()->getSavedValue<bool>("speedhack", false);
        speedhack_classic = geode::Mod::get()->getSavedValue<bool>("speedhack-classic", false);
        speedhack_multiplier = geode::Mod::get()->getSavedValue<float>("speedhack-multiplier", 1.0f);
        lock_delta = geode::Mod::get()->getSavedValue<bool>("lock-delta", false);
        noclip = geode::Mod::get()->getSavedValue<bool>("noclip", false);
        noclip_p1 = geode::Mod::get()->getSavedValue<bool>("noclip-p1", false);
        noclip_p2 = geode::Mod::get()->getSavedValue<bool>("noclip-p2", false);
        hitboxes = geode::Mod::get()->getSavedValue<bool>("hitboxes", false);
        hitboxes_trajectory = geode::Mod::get()->getSavedValue<bool>("hitboxes-trajectory", false);
        hitboxes_trajectory_length = geode::Mod::get()->getSavedValue<int>("hitboxes-trajectory-length", 100);
        hitboxes_thickness = geode::Mod::get()->getSavedValue<float>("hitboxes-thickness", 0.3f);
        hitboxes_color_hazards = geode::Mod::get()->getSavedValue<std::vector<float>>("hitboxes-color-hazards", { 1.0f, 0.0f, 0.0f, 1.0f });
        hitboxes_color_fill_hazards = geode::Mod::get()->getSavedValue<std::vector<float>>("hitboxes-color-fill-hazards", { 1.0f, 0.0f, 0.0f, 0.2f });
        hitboxes_color_solids = geode::Mod::get()->getSavedValue<std::vector<float>>("hitboxes-color-solids", { 0.0f, 0.0f, 1.0f, 1.0f });
        hitboxes_color_fill_solids = geode::Mod::get()->getSavedValue<std::vector<float>>("hitboxes-color-fill-solids", { 0.0f, 0.0f, 1.0f, 0.2f });
        hitboxes_color_specials = geode::Mod::get()->getSavedValue<std::vector<float>>("hitboxes-color-specials", { 0.0f, 1.0f, 0.0f, 1.0f });
        hitboxes_color_fill_specials = geode::Mod::get()->getSavedValue<std::vector<float>>("hitboxes-color-fill-specials", { 0.0f, 1.0f, 0.0f, 0.2f });
        hitboxes_color_player = geode::Mod::get()->getSavedValue<std::vector<float>>("hitboxes-color-player", { 1.0f, 1.0f, 0.0f, 1.0f });
        hitboxes_color_fill_player = geode::Mod::get()->getSavedValue<std::vector<float>>("hitboxes-color-fill-player", { 1.0f, 1.0f, 0.0f, 0.2f });
        layout_mode = geode::Mod::get()->getSavedValue<bool>("layout-mode", false);
        practice_fix = geode::Mod::get()->getSavedValue<bool>("practice-fix", true);
        copy_hack = geode::Mod::get()->getSavedValue<bool>("copy-hack", false);
    }
}

static bool debug_update = false;
static std::chrono::steady_clock::time_point last_time, now;
static std::chrono::steady_clock::duration accumulator;

#define POLYGON_FROM_RECT(r) { cocos2d::CCPoint((r).getMinX(), (r).getMinY()), cocos2d::CCPoint((r).getMaxX(), (r).getMinY()), cocos2d::CCPoint((r).getMaxX(), (r).getMaxY()), cocos2d::CCPoint((r).getMinX(), (r).getMaxY()) }

class $modify(GJBaseGameLayer) {
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
        
        this->addObject(obj);
    }
    
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

class $modify(PlayLayer) {
    void updateProgressbar(void) {
        PlayLayer::updateProgressbar();

        // 0166 -> Is "Show Hitboxes" option on?
        if (uv::hacks::hitboxes && !this->m_isPracticeMode || !GameManager::get()->getGameVariable("0166")) PlayLayer::updateDebugDraw();
        this->m_debugDrawNode->setVisible(uv::hacks::hitboxes);
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

class $modify(LevelInfoLayer) {
    LevelInfoLayer *create(GJGameLevel *level, bool challenge) {
        if (uv::hacks::copy_hack) m_level->m_password = 1;

        return LevelInfoLayer::create(level, challenge);
    }

    void tryCloneLevel(cocos2d::CCObject *sender) {
        if (uv::hacks::copy_hack) m_level->m_password = 1;
        LevelInfoLayer::tryCloneLevel(sender);
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
