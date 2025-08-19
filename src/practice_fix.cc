#include "common.hh"

namespace uv::practice_fix {
    checkpoint_data from_playerobject(PlayerObject *po) {
        checkpoint_data data;
        memcpy(data.data, po, sizeof(PlayerObject));
        data.rotation = po->getRotation();
        return data;
    }
    
    void restore_playerobject(PlayerObject *po, checkpoint_data cpd) {
        memcmp(po, cpd.data, sizeof(PlayerObject));
        po->setRotation(cpd.rotation);
    }
}
