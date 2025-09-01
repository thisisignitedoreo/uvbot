#include <Geode/Geode.hpp>

#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GameObject.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/CCDrawNode.hpp>
#include <Geode/modify/CCScheduler.hpp>

#include "common.hh"

static bool debug_update = false;
static std::chrono::steady_clock::time_point last_time, now;
static std::chrono::steady_clock::duration accumulator;
static float left_over = 0.0f;

#define POLYGON_FROM_RECT(r) { cocos2d::CCPoint((r).getMinX(), (r).getMinY()), cocos2d::CCPoint((r).getMaxX(), (r).getMinY()), cocos2d::CCPoint((r).getMaxX(), (r).getMaxY()), cocos2d::CCPoint((r).getMinX(), (r).getMaxY()) }

class $modify(GJBaseGameLayer) {
    void addObject(GameObject *obj) {
        if (uv::hacks::get("layout-mode", false)) {
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

    // Thanks zilko
    float getModifiedDelta(float delta) {
        if (uv::hacks::get("tps", 240.0f) <= 240.0f) return GJBaseGameLayer::getModifiedDelta(delta);
        
        if (this->m_resumeTimer > 0) {
            this->m_resumeTimer--;
            delta = 0.0f;
        }

        // I have no idea
        float timewarp = std::min(this->m_gameState.m_timeWarp, 1.0f);
        float new_delta = 1.0f / uv::hacks::get("tps", 240.0f);
        double var1 = delta + this->m_extraDelta;
        float var2 = std::round(var1 / (timewarp * new_delta));
        double var3 = var2 * timewarp * new_delta;
        
        m_extraDelta = var1 - var3;

        return var3;
    }
};

class $modify(PlayLayer) {
    void updateProgressbar(void) {
        PlayLayer::updateProgressbar();

        // 0166 -> Is "Show Hitboxes" option on?
        if (uv::hacks::get("hitboxes", false) && !this->m_isPracticeMode || !GameManager::get()->getGameVariable("0166")) PlayLayer::updateDebugDraw();
        this->m_debugDrawNode->setVisible(uv::hacks::get("hitboxes", false));
    }

    // Thanks toby
    void updateColor(cocos2d::ccColor3B &color, float fadeTime, int colorID, bool blending, float opacity, cocos2d::ccHSVValue &copyHSV, int colorIDToCopy, bool copyOpacity, EffectGameObject *callerObject, int unk1, int unk2) {
        cocos2d::ccColor3B copy_color = color;
        if (uv::hacks::get("layout-mode", false)) {
            if (colorID == 1000) copy_color = {0, 0, 0};
            else if (colorID == 1001) copy_color = {0, 0, 0};
            else if (colorID == 1002) copy_color = {255, 255, 255};
            else if (colorID == 1009) copy_color = {0, 0, 0};
            else if (colorID == 1013 || colorID == 1014) copy_color = {0, 0, 0};
            else copy_color = {255, 255, 255};
        }

        PlayLayer::updateColor(copy_color, fadeTime, colorID, blending, opacity, copyHSV, colorIDToCopy, copyOpacity, callerObject, unk1, unk2);
    }
    
    void update(float delta) {
        if (uv::hacks::get("tps", 240.0f) <= 240.0f) { PlayLayer::update(delta); return; }

        float target_delta = 1.0f / uv::hacks::get("tps", 240.0f);
        float real_delta = delta + left_over;
        if (real_delta > delta && target_delta < delta) real_delta = delta;

        auto start = std::chrono::high_resolution_clock::now();
        int multiplier = static_cast<int>(real_delta / target_delta);

        for (int i = 0; i < multiplier; i++) {
            PlayLayer::update(target_delta);
            if (std::chrono::high_resolution_clock::now() - start > std::chrono::duration<double, std::milli>(16.666f)) {
                multiplier = i + 1;
                break;
            }
        }
        
        left_over += (delta - target_delta * multiplier);
    }

    // NOTE: to achive TPS lower than 240.0f you need to midhook update
    // Geode can not do that right now :(
};

class $modify(GameObject) {
    void addGlow(gd::string p0) {
        GameObject::addGlow(p0);
        if (uv::hacks::get("layout-mode", false) && this->m_objectType == GameObjectType::Decoration) this->m_isHide = true;
    }

    // Thanks cvolton, unthanks robtop
    void determineSlopeDirection() {
        if (debug_update) return;
        GameObject::determineSlopeDirection();
    }
};

class $modify(cocos2d::CCScheduler) {
    void update(float dt) {
        if (uv::hacks::get("speedhack", false) && !uv::hacks::get("speedhack-classic", false)) dt *= uv::hacks::get("speedhack-multiplier", 1.0f);
        CCScheduler::update(dt);
    }
};

class $modify(LevelInfoLayer) {
    bool init(GJGameLevel *level, bool challenge) {
        if (uv::hacks::get("copy-hack", false)) level->m_password = 1;
        
        return LevelInfoLayer::init(level, challenge);
    }
};

class $modify(cocos2d::CCDirector) {
    bool init(void) {
        if (!cocos2d::CCDirector::init()) return false;
        last_time = std::chrono::steady_clock::now();
        return true;
    }
    
    void drawScene(void) {
        if (uv::hacks::get("speedhack", false) && uv::hacks::get("speedhack-classic", false)) {
            now = std::chrono::steady_clock::now();
            accumulator += now - last_time;
            last_time = now;

            double frame_length = this->getAnimationInterval() * (1.0 / uv::hacks::get("speedhack-multiplier", 1.0f));
            
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
        thickness = uv::hacks::get("hitboxes-thickness", 0.3f);

        cocos2d::ccColor4F copy_color, copy_fill_color;
        if (COLOR4F_EQUALS(borderColor, 1.0f, 0.0f, 0.0f)) {
            copy_color = FLOAT4_TO_COLOR4F(uv::hacks::get<std::vector<float>>("hitboxes-color-hazards", { 1.0f, 0.0f, 0.0f, 1.0f }));
            copy_fill_color = FLOAT4_TO_COLOR4F(uv::hacks::get<std::vector<float>>("hitboxes-color-fill-hazards", { 1.0f, 0.0f, 0.0f, 0.2f }));
        } else if (COLOR4F_EQUALS(borderColor, 0.0f, 0.25f, 1.0f)) {
            copy_color = FLOAT4_TO_COLOR4F(uv::hacks::get<std::vector<float>>("hitboxes-color-solids", { 0.0f, 0.0f, 1.0f, 1.0f }));
            copy_fill_color = FLOAT4_TO_COLOR4F(uv::hacks::get<std::vector<float>>("hitboxes-color-fill-solids", { 0.0f, 0.0f, 1.0f, 0.2f }));
        } else if (COLOR4F_EQUALS(borderColor, 0.0f, 1.0f, 0.0f)) {
            copy_color = FLOAT4_TO_COLOR4F(uv::hacks::get<std::vector<float>>("hitboxes-color-specials", { 0.0f, 1.0f, 0.0f, 1.0f }));
            copy_fill_color = FLOAT4_TO_COLOR4F(uv::hacks::get<std::vector<float>>("hitboxes-color-fill-specials", { 0.0f, 1.0f, 0.0f, 0.2f }));
        } else if (COLOR4F_EQUALS(borderColor, 1.0f, 1.0f, 0.0f)) {
            copy_color = FLOAT4_TO_COLOR4F(uv::hacks::get<std::vector<float>>("hitboxes-color-player", { 1.0f, 1.0f, 0.0f, 1.0f }));
            copy_fill_color = FLOAT4_TO_COLOR4F(uv::hacks::get<std::vector<float>>("hitboxes-color-fill-player", { 1.0f, 1.0f, 0.0f, 0.2f }));
        } else {
            copy_color = borderColor;
            copy_fill_color = fillColor;
        }

        return cocos2d::CCDrawNode::drawPolygon(vertex, count, copy_fill_color, thickness, copy_color);
    }
    
    bool drawCircle(const cocos2d::CCPoint &position, float radius, const cocos2d::ccColor4F &color, float thickness, const cocos2d::ccColor4F &borderColor, unsigned int segments) {
        thickness = uv::hacks::get("hitboxes-thickness", 0.3f);
        this->m_bUseArea = false;

        cocos2d::ccColor4F copy_color, copy_fill_color;
        if (COLOR4F_EQUALS(borderColor, 1.0f, 0.0f, 0.0f)) {
            copy_color = FLOAT4_TO_COLOR4F(uv::hacks::get<std::vector<float>>("hitboxes-color-hazards", { 1.0f, 0.0f, 0.0f, 1.0f }));
            copy_fill_color = FLOAT4_TO_COLOR4F(uv::hacks::get<std::vector<float>>("hitboxes-color-fill-hazards", { 1.0f, 0.0f, 0.0f, 0.2f }));
        } else if (COLOR4F_EQUALS(borderColor, 0.0f, 0.25f, 1.0f)) {
            copy_color = FLOAT4_TO_COLOR4F(uv::hacks::get<std::vector<float>>("hitboxes-color-solids", { 0.0f, 0.0f, 1.0f, 1.0f }));
            copy_fill_color = FLOAT4_TO_COLOR4F(uv::hacks::get<std::vector<float>>("hitboxes-color-fill-solids", { 0.0f, 0.0f, 1.0f, 0.2f }));
        } else if (COLOR4F_EQUALS(borderColor, 0.0f, 1.0f, 0.0f)) {
            copy_color = FLOAT4_TO_COLOR4F(uv::hacks::get<std::vector<float>>("hitboxes-color-specials", { 0.0f, 1.0f, 0.0f, 1.0f }));
            copy_fill_color = FLOAT4_TO_COLOR4F(uv::hacks::get<std::vector<float>>("hitboxes-color-fill-specials", { 0.0f, 1.0f, 0.0f, 0.2f }));
        } else if (COLOR4F_EQUALS(borderColor, 1.0f, 1.0f, 0.0f)) {
            copy_color = FLOAT4_TO_COLOR4F(uv::hacks::get<std::vector<float>>("hitboxes-color-player", { 1.0f, 1.0f, 0.0f, 1.0f }));
            copy_fill_color = FLOAT4_TO_COLOR4F(uv::hacks::get<std::vector<float>>("hitboxes-color-fill-player", { 1.0f, 1.0f, 0.0f, 0.2f }));
        } else {
            copy_color = borderColor;
            copy_fill_color = color;
        }

        return cocos2d::CCDrawNode::drawCircle(position, radius, copy_fill_color, thickness, copy_color, segments);
    }
};
