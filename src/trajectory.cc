#include <Geode/Geode.hpp>

#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

#include "common.hh"

#define FLOAT4_TO_COLOR4F(a) (cocos2d::ccColor4F { (a)[0], (a)[1], (a)[2], (a)[3] })
#define POLYGON_FROM_RECT(r) { cocos2d::CCPoint((r).getMinX(), (r).getMinY()), cocos2d::CCPoint((r).getMaxX(), (r).getMinY()), cocos2d::CCPoint((r).getMaxX(), (r).getMaxY()), cocos2d::CCPoint((r).getMinX(), (r).getMaxY()) }
#define CORNERS_TO_ARR(a) { (a)[0], (a)[1], (a)[2], (a)[3] }

namespace uv::hacks::trajectory {
    bool making_trajectory = false;
    bool trajectory_end = false;
    
    static void update_player(GJBaseGameLayer *gjbgl, PlayerObject *player, cocos2d::CCDrawNode *node, bool hold) {
        float delta = 1 / 4.0f;
        
        if (hold && !player->m_holdingButtons[0]) player->pushButton(PlayerButton::Jump);
        else if (!hold && player->m_holdingButtons[0]) player->releaseButton(PlayerButton::Jump);

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

class $modify(GJBaseGameLayer) {
    struct Fields {
        PlayerObject *trajectory_players[2];
        cocos2d::CCDrawNode *trajectory_node;
    };
    
    void processCommands(float dt) {
        GJBaseGameLayer::processCommands(dt);
        if (uv::hacks::hitboxes_trajectory) {
        }
    }
    
    bool canBeActivatedByPlayer(PlayerObject *p0, EffectGameObject *p1) {
        if (uv::hacks::trajectory::making_trajectory) return false;
        return GJBaseGameLayer::canBeActivatedByPlayer(p0, p1);
    }
    
    void playerTouchedRing(PlayerObject *p0, RingObject *p1) {
        if (!uv::hacks::trajectory::making_trajectory) GJBaseGameLayer::playerTouchedRing(p0, p1);
    }

    void playerTouchedTrigger(PlayerObject *player, EffectGameObject *p1) {
        if (!uv::hacks::trajectory::making_trajectory) GJBaseGameLayer::playerTouchedTrigger(player, p1);
        else {
            // Thanks Zilko
            switch (p1->m_objectID) {
            case 101:
            case 99:
            case 200:
            case 201:
            case 202:
            case 203:
            case 1334:
                GJBaseGameLayer::playerTouchedTrigger(player, p1);
            }
        }
    }
    
    void collisionCheckObjects(PlayerObject *p0, gd::vector<GameObject*> *objects, int p2, float p3) {
        if (uv::hacks::trajectory::making_trajectory) {
            std::vector<GameObject*> to_reenable;

            for (GameObject *obj : *objects) {
                if (!obj) continue;

                switch (obj->m_objectType) {
                case GameObjectType::Collectible:
                case GameObjectType::UserCoin:
                case GameObjectType::SecretCoin: {
                    if (obj->m_isDisabled || obj->m_isDisabled2) break;

                    to_reenable.push_back(obj);
                    obj->m_isDisabled = true;
                    obj->m_isDisabled2 = true;
                } break;
                }
            }

            GJBaseGameLayer::collisionCheckObjects(p0, objects, p2, p3);

            for (GameObject *obj : to_reenable) {
                if (!obj) continue;

                obj->m_isDisabled = false;
                obj->m_isDisabled2 = false;
            }
        } else GJBaseGameLayer::collisionCheckObjects(p0, objects, p2, p3);
    }

    void activateSFXTrigger(SFXTriggerGameObject *p0) {
        if (!uv::hacks::trajectory::making_trajectory) GJBaseGameLayer::activateSFXTrigger(p0);

    }
    void activateSongEditTrigger(SongTriggerGameObject *p0) {
        if (!uv::hacks::trajectory::making_trajectory) GJBaseGameLayer::activateSongEditTrigger(p0);
    }
    
    void gameEventTriggered(GJGameEvent p0, int p1, int p2) {
        if (!uv::hacks::trajectory::making_trajectory) GJBaseGameLayer::gameEventTriggered(p0, p1, p2);
    }
    
    void update(float dt) {
        GJBaseGameLayer::update(dt);

        if (!m_fields->trajectory_players[0] || !m_fields->trajectory_players[1] || !m_fields->trajectory_node) {
            m_fields->trajectory_players[0] = PlayerObject::create(1, 1, this, this, true);
            m_fields->trajectory_players[1] = PlayerObject::create(1, 1, this, this, true);
            m_fields->trajectory_node = cocos2d::CCDrawNode::create();

            m_fields->trajectory_players[0]->setVisible(false);
            m_fields->trajectory_players[1]->setVisible(false);
            m_fields->trajectory_node->setVisible(true);
            
            cocos2d::_ccBlendFunc blendFunc;
            blendFunc.src = GL_SRC_ALPHA;
            blendFunc.dst = GL_ONE_MINUS_SRC_ALPHA;

            m_fields->trajectory_node->setBlendFunc(blendFunc);
            
            this->m_objectLayer->addChild(m_fields->trajectory_players[0]);
            this->m_objectLayer->addChild(m_fields->trajectory_players[1]);
            this->m_debugDrawNode->getParent()->addChild(m_fields->trajectory_node, 1402, -9998);
        }

        if (uv::hacks::hitboxes_trajectory) {
            m_fields->trajectory_node->clear();
            
            m_fields->trajectory_players[0]->copyAttributes(this->m_player1);
            m_fields->trajectory_players[1]->copyAttributes(this->m_player1);
            m_fields->trajectory_players[0]->setRotation(this->m_player1->getRotation());
            m_fields->trajectory_players[1]->setRotation(this->m_player1->getRotation());
            m_fields->trajectory_players[0]->m_yVelocity = this->m_player1->m_yVelocity;
            m_fields->trajectory_players[1]->m_yVelocity = this->m_player1->m_yVelocity;
        
            uv::hacks::trajectory::update(this, m_fields->trajectory_players, m_fields->trajectory_node);

            if (this->m_player2) {
                m_fields->trajectory_players[0]->copyAttributes(this->m_player2);
                m_fields->trajectory_players[1]->copyAttributes(this->m_player2);
                m_fields->trajectory_players[0]->setRotation(this->m_player2->getRotation());
                m_fields->trajectory_players[1]->setRotation(this->m_player2->getRotation());
                m_fields->trajectory_players[0]->m_yVelocity = this->m_player2->m_yVelocity;
                m_fields->trajectory_players[1]->m_yVelocity = this->m_player2->m_yVelocity;
                
                uv::hacks::trajectory::update(this, m_fields->trajectory_players, m_fields->trajectory_node);
            }
        }
    }
};

class $modify(PlayLayer) {
    void destroyPlayer(PlayerObject *po, GameObject *go) {
        if (uv::hacks::trajectory::making_trajectory) {
            uv::hacks::trajectory::trajectory_end = true;
            return;
        }

        bool let_him_live = uv::hacks::noclip || (uv::hacks::noclip_p1 && po == this->m_player1) || (uv::hacks::noclip_p2 && po == this->m_player2);
        if (!let_him_live) PlayLayer::destroyPlayer(po, go);
    }

    void updateProgressbar(void) {
        PlayLayer::updateProgressbar();
        
        cocos2d::CCNode *trajectory_node = this->m_debugDrawNode->getParent()->getChildByTag(-9998);
        if (trajectory_node) trajectory_node->setVisible(uv::hacks::hitboxes && uv::hacks::hitboxes_trajectory);
    }
};

class $modify(PlayerObject) {
    void playSpiderDashEffect(cocos2d::CCPoint p0, cocos2d::CCPoint p1) {
        if (!uv::hacks::trajectory::making_trajectory) PlayerObject::playSpiderDashEffect(p0, p1);
    }
};
