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
