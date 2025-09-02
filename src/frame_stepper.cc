#include <Geode/Geode.hpp>

#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include "common.hh"

namespace uv::hacks::frame_stepper {
    bool on = false;
    int step_for = 0;
}

/*class $modify(cocos2d::CCDirector) {
    void drawScene(void) {
        bool fs_on = uv::hacks::get<bool>("frame-stepper", false);
        if (!fs_on) return CCDirector::drawScene();

        PlayLayer *pl = PlayLayer::get();
        if (!pl) return CCDirector::drawScene();

        if (pl->m_isPaused || pl->m_player1->m_isDead) return CCDirector::drawScene();
        if (uv::bot::get_frame() < 3) return CCDirector::drawScene();

        if (uv::hacks::frame_stepper::step_for > 0) {
            CCDirector::drawScene();
        } else this->getScheduler()->update(1.0f / uv::hacks::get<float>("tps", 240.0f));
    }
};*/

class $modify(GJBaseGameLayer) {
    void update(float dt) {
        if (!PlayLayer::get()) return GJBaseGameLayer::update(dt);

        if (uv::hacks::frame_stepper::on) {
            if (uv::hacks::frame_stepper::step_for) {
                uv::hacks::frame_stepper::step_for--;
                GJBaseGameLayer::update(1.f / uv::hacks::get<float>("tps", 240.0f));
            }
        } else GJBaseGameLayer::update(dt);
    }
};

class $modify(PlayLayer) {
    void playEndAnimationToPos(cocos2d::CCPoint p1) {
        PlayLayer::playEndAnimationToPos(p1);
        uv::hacks::frame_stepper::on = false;
    }
};
