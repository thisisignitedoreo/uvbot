#include <Geode/Geode.hpp>

#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/CheckpointObject.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include "common.hh"

namespace uv::practice_fix {
    checkpoint_data from_playerobject(PlayerObject *po) {
        checkpoint_data data;
        data.y_velocity = po->m_yVelocity;
        data.rotation = po->getObjectRotation();
        data.position = po->getPosition();
        return data;
    }
    
    void restore_playerobject(PlayerObject *po, checkpoint_data data) {
        po->m_yVelocity = data.y_velocity;
        po->setRotation(data.rotation);
        po->setPosition(data.position);
    }
}

static bool override_rotation = false;

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

class $modify(PlayLayer) {
    void loadFromCheckpoint(CheckpointObject *cp) {
        PlayLayer::loadFromCheckpoint(cp);

        if (uv::hacks::practice_fix) {
            uv::practice_fix::restore_playerobject(this->m_player1, static_cast<HookedCheckpointObject*>(cp)->m_fields->p1);
            uv::practice_fix::restore_playerobject(this->m_player2, static_cast<HookedCheckpointObject*>(cp)->m_fields->p2);
            override_rotation = true;
        }
    }
};
