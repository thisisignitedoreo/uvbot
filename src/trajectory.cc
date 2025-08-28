#include <Geode/Geode.hpp>

#include "common.hh"

#define FLOAT4_TO_COLOR4F(a) (cocos2d::ccColor4F { (a)[0], (a)[1], (a)[2], (a)[3] })
#define POLYGON_FROM_RECT(r) { cocos2d::CCPoint((r).getMinX(), (r).getMinY()), cocos2d::CCPoint((r).getMaxX(), (r).getMinY()), cocos2d::CCPoint((r).getMaxX(), (r).getMaxY()), cocos2d::CCPoint((r).getMinX(), (r).getMaxY()) }
#define CORNERS_TO_ARR(a) { (a)[0], (a)[1], (a)[2], (a)[3] }

namespace uv::hacks::trajectory {
    bool making_trajectory = false;
    bool trajectory_end = false;
    
    static void update_player(GJBaseGameLayer *gjbgl, PlayerObject *player, cocos2d::CCDrawNode *node, bool hold) {
        float delta = 1 / 4.0f;
        
        if (hold) player->pushButton(PlayerButton::Jump);
        else player->releaseButton(PlayerButton::Jump);

        cocos2d::CCRect box;
        OBB2D *oriented_box;

        for (int i = 0; i < uv::hacks::hitboxes_trajectory_length; i++) {
            cocos2d::CCPoint previous = player->getPosition();

            player->m_collisionLogTop->removeAllObjects();
            player->m_collisionLogBottom->removeAllObjects();
            player->m_collisionLogLeft->removeAllObjects();
            player->m_collisionLogRight->removeAllObjects();

            gjbgl->checkCollisions(player, delta, false);
            
            box = player->getObjectRect();
            oriented_box = player->getOrientedBox();

            if (trajectory_end) {
                player->updatePlayerScale();
                trajectory_end = false;
                break;
            }

            player->update(delta);
            player->updateRotation(delta);
            player->updatePlayerScale();

            cocos2d::CCPoint current = player->getPosition();

            node->drawSegment(previous, current, uv::hacks::hitboxes_thickness, FLOAT4_TO_COLOR4F(uv::hacks::hitboxes_color_player));
        }
        
        if (hold) player->releaseButton(PlayerButton::Jump);

        const cocos2d::ccColor4F blank {0.0f, 0.0f, 0.0f, 0.0f};
        const cocos2d::ccColor4F yellow {1.0f, 1.0f, 0.0f, 1.0f};
        
        cocos2d::CCPoint polygon[] = POLYGON_FROM_RECT(box);
        node->drawPolygon(polygon, 4, blank, 1, yellow);
        
        cocos2d::CCPoint polygon_2[] = CORNERS_TO_ARR(oriented_box->m_corners);
        node->drawPolygon(polygon_2, 4, blank, 1, yellow);
    }
    
    void update(GJBaseGameLayer *gjbgl, PlayerObject *trajectory_players[2], cocos2d::CCDrawNode *trajectory_node) {
        making_trajectory = true;
        update_player(gjbgl, trajectory_players[0], trajectory_node, false);
        update_player(gjbgl, trajectory_players[1], trajectory_node, true);
        making_trajectory = false;
    }
}
