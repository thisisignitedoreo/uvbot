#pragma once

#include <chrono>

#include <Geode/Geode.hpp>

namespace uv {
    const std::filesystem::path macro_path = geode::Mod::get()->getSaveDir() / "Macros";
    const std::filesystem::path showcase_path = geode::Mod::get()->getSaveDir() / "Showcases";
    extern std::vector<std::string> installed_conflict_mods;
    
    namespace gui {
        extern bool show;
        extern bool debug;
        extern std::chrono::time_point<std::chrono::steady_clock> toggle_time;
        
        void setup(void);
        void draw(void);
    }

    namespace bot {
        // Frame type (u64)
        typedef unsigned long long frame_t;
        
        struct input_action {
            frame_t frame;
            unsigned char flags;
        };

        struct physic_action {
            frame_t frame;
            float x, y, rotation;
            double y_velocity;
        };

        extern std::vector<input_action> input_actions;
        extern std::vector<physic_action> physic_player_1_actions;
        extern std::vector<physic_action> physic_player_2_actions;

        extern int current_input_action;
        extern int current_physic_player_1_action;
        extern int current_physic_player_2_action;
        
        enum state: int {
            none = 0,
            recording,
            playing,
        };
        
        extern state current_state;
        
        extern float record_tps;
        
        frame_t get_frame(void);
        
        void reset(void);
        bool button(bool held, int button, bool player_1);
        void update_input(GJBaseGameLayer *self, float delta);
        void update_physics(GJBaseGameLayer *self, float delta);

        void clear(void);
        void save(std::string name);
        void load(std::string name);

        namespace foreign {
            enum error {
                none = 0,
                unsupported,
                unsupported_slc_v2,
                unsupported_slc_v3,
            };
            
            extern std::vector<std::string> supported_exts;

            error load(std::string name);
        }
    }
    
    namespace recorder {
        struct options {
            int width, height;
            float fps, excess_render;
            std::string codec, bitrate;
            std::string output_path;
            std::string custom_options;
            bool hide_end_level_screen, fade_out;
        };

        void start(options opt);
        void update(void);
        void end(void);

        extern bool recording;
        extern options recording_options;
        
        namespace audio {
            struct options {
                std::string output_path;
                float music_volume, sfx_volume;
                float excess_render;
            };
            
            void init(options opt);
            void start(void);
            void end(void);

            extern bool recording;
            extern options recording_options;
        }

        void merge(std::string video, std::string audio, std::string output, std::string args);
        void raw(std::string args);
        bool process_done(void);
    }
    
    namespace hacks {
        template <typename T = bool>
        T get(std::string id, T d) {
            return geode::Mod::get()->getSavedValue<T>(id, d);
        }
        
        template <typename T = bool>
        T set(std::string id, T d) {
            return geode::Mod::get()->setSavedValue<T>(id, d);
        }

        namespace trajectory {
            extern bool making_trajectory, trajectory_end;
            
            void update(GJBaseGameLayer *gjbgl, PlayerObject *trajectory_players[2], cocos2d::CCDrawNode *trajectory_node);
        }

        namespace frame_stepper {
            extern bool on;
            extern int step_for;
        }
    }
    
    namespace practice_fix {
        struct checkpoint_data {
            double rotation;
            // Copied directly from Geode bindings
            // https://github.com/geode-sdk/bindings/blob/main/bindings/2.2074/GeometryDash.bro#L18490
            cocos2d::CCNode* m_mainLayer;
            bool m_wasTeleported;
            bool m_fixGravityBug;
            bool m_reverseSync;
            double m_yVelocityBeforeSlope;
            double m_dashX;
            double m_dashY;
            double m_dashAngle;
            double m_dashStartTime;
            DashRingObject* m_dashRing;
            double m_slopeStartTime;
            bool m_justPlacedStreak;
            cocos2d::CCNode* m_maybeLastGroundObject;
            cocos2d::CCDictionary* m_collisionLogTop;
            cocos2d::CCDictionary* m_collisionLogBottom;
            cocos2d::CCDictionary* m_collisionLogLeft;
            cocos2d::CCDictionary* m_collisionLogRight;
            int m_lastCollisionBottom;
            int m_lastCollisionTop;
            int m_lastCollisionLeft;
            int m_lastCollisionRight;
            int m_unk50C;
            int m_unk510;
            GameObject* m_currentSlope2;
            GameObject* m_preLastGroundObject;
            float m_slopeAngle;
            bool m_slopeSlidingMaybeRotated;
            bool m_quickCheckpointMode;
            GameObject* m_collidedObject;
            GameObject* m_lastGroundObject;
            GameObject* m_collidingWithLeft;
            GameObject* m_collidingWithRight;
            int m_maybeSavedPlayerFrame;
            double m_scaleXRelated2;
            double m_groundYVelocity;
            double m_yVelocityRelated;
            double m_scaleXRelated3;
            double m_scaleXRelated4;
            double m_scaleXRelated5;
            bool m_isCollidingWithSlope;
            cocos2d::CCSprite* m_dashFireSprite;
            bool m_isBallRotating;
            bool m_unk669;
            GameObject* m_currentSlope3;
            GameObject* m_currentSlope;
            double unk_584;
            int m_collidingWithSlopeId;
            bool m_slopeFlipGravityRelated;
            cocos2d::CCArray* m_particleSystems;
            float m_slopeAngleRadians;
            gd::unordered_map<int, GJPointDouble> m_rotateObjectsRelated;
            gd::unordered_map<int, GameObject*> m_maybeRotatedObjectsMap;
            float m_rotationSpeed;
            float m_rotateSpeed;
            bool m_isRotating;
            bool m_isBallRotating2;
            bool m_hasGlow;
            bool m_isHidden;
            GhostType m_ghostType;
            GhostTrailEffect* m_ghostTrail;
            cocos2d::CCSprite* m_iconSprite;
            cocos2d::CCSprite* m_iconSpriteSecondary;
            cocos2d::CCSprite* m_iconSpriteWhitener;
            cocos2d::CCSprite* m_iconGlow;
            cocos2d::CCSprite* m_vehicleSprite;
            cocos2d::CCSprite* m_vehicleSpriteSecondary;
            cocos2d::CCSprite* m_birdVehicle;
            cocos2d::CCSprite* m_vehicleSpriteWhitener;
            cocos2d::CCSprite* m_vehicleGlow;
            PlayerFireBoostSprite* m_swingFireMiddle;
            PlayerFireBoostSprite* m_swingFireBottom;
            PlayerFireBoostSprite* m_swingFireTop;
            cocos2d::CCSprite* m_dashSpritesContainer;
            cocos2d::CCMotionStreak* m_regularTrail;
            cocos2d::CCMotionStreak* m_shipStreak;
            HardStreak* m_waveTrail;
            double m_speedMultiplier;
            double m_yStart;
            double m_gravity;
            float m_trailingParticleLife;
            float m_unk648;
            double m_gameModeChangedTime;
            bool m_padRingRelated;
            bool m_maybeReducedEffects;
            bool m_maybeIsFalling;
            bool m_shouldTryPlacingCheckpoint;
            bool m_playEffects;
            bool m_maybeCanRunIntoBlocks;
            bool m_hasGroundParticles;
            bool m_hasShipParticles;
            bool m_isOnGround3;
            bool m_checkpointTimeout;
            double m_lastCheckpointTime;
            double m_lastJumpTime;
            double m_lastFlipTime;
            double m_flashTime;
            float m_flashRelated;
            float m_flashRelated1;
            cocos2d::ccColor3B m_colorRelated2;
            cocos2d::ccColor3B m_flashRelated3;
            double m_lastSpiderFlipTime;
            bool m_unkBool5;
            bool m_maybeIsVehicleGlowing;
            bool m_switchWaveTrailColor;
            bool m_practiceDeathEffect;
            double m_accelerationOrSpeed;
            double m_snapDistance;
            bool m_ringJumpRelated;
            gd::unordered_set<int> m_ringRelatedSet;
            GameObject* m_objectSnappedTo;
            CheckpointObject* m_pendingCheckpoint;
            int m_onFlyCheckpointTries;
            GJRobotSprite* m_robotSprite;
            GJSpiderSprite* m_spiderSprite;
            bool m_maybeSpriteRelated;
            cocos2d::CCParticleSystemQuad* m_playerGroundParticles;
            cocos2d::CCParticleSystemQuad* m_trailingParticles;
            cocos2d::CCParticleSystemQuad* m_shipClickParticles;
            cocos2d::CCParticleSystemQuad* m_vehicleGroundParticles;
            cocos2d::CCParticleSystemQuad* m_ufoClickParticles;
            cocos2d::CCParticleSystemQuad* m_robotBurstParticles;
            cocos2d::CCParticleSystemQuad* m_dashParticles;
            cocos2d::CCParticleSystemQuad* m_swingBurstParticles1;
            cocos2d::CCParticleSystemQuad* m_swingBurstParticles2;
            bool m_useLandParticles0;
            cocos2d::CCParticleSystemQuad* m_landParticles0;
            cocos2d::CCParticleSystemQuad* m_landParticles1;
            float m_landParticlesAngle;
            float m_landParticleRelatedY;
            int m_playerStreak;
            float m_streakStrokeWidth;
            bool m_disableStreakTint;
            bool m_alwaysShowStreak;
            ShipStreak m_shipStreakType;
            double m_slopeRotation;
            double m_currentSlopeYVelocity;
            double m_unk3d0;
            double m_blackOrbRelated;
            bool m_unk3e0;
            bool m_unk3e1;
            bool m_isAccelerating;
            bool m_isCurrentSlopeTop;
            double m_collidedTopMinY;
            double m_collidedBottomMaxY;
            double m_collidedLeftMaxX;
            double m_collidedRightMinX;
            bool m_fadeOutStreak;
            bool m_canPlaceCheckpoint;
            cocos2d::ccColor3B m_colorRelated;
            cocos2d::ccColor3B m_secondColorRelated;
            bool m_hasCustomGlowColor;
            cocos2d::ccColor3B m_glowColor;
            bool m_maybeIsColliding;
            bool m_jumpBuffered;
            bool m_stateRingJump;
            bool m_wasJumpBuffered;
            bool m_wasRobotJump;
            unsigned char m_stateJumpBuffered;
            bool m_stateRingJump2;
            bool m_touchedRing;
            bool m_touchedCustomRing;
            bool m_touchedGravityPortal;
            bool m_maybeTouchedBreakableBlock;
            geode::SeedValueRSV m_jumpRelatedAC2;
            bool m_touchedPad;
            double m_yVelocity;
            double m_fallSpeed;
            bool m_isOnSlope;
            bool m_wasOnSlope;
            float m_slopeVelocity;
            bool m_maybeUpsideDownSlope;
            bool m_isShip;
            bool m_isBird;
            bool m_isBall;
            bool m_isDart;
            bool m_isRobot;
            bool m_isSpider;
            bool m_isUpsideDown;
            bool m_isDead;
            bool m_isOnGround;
            bool m_isGoingLeft;
            bool m_isSideways;
            bool m_isSwing;
            int m_reverseRelated;
            double m_maybeReverseSpeed;
            double m_maybeReverseAcceleration;
            float m_xVelocityRelated2;
            bool m_isDashing;
            int m_unk9e8;
            int m_groundObjectMaterial;
            float m_vehicleSize;
            float m_playerSpeed;
            cocos2d::CCPoint m_shipRotation;
            cocos2d::CCPoint m_lastPortalPos;
            float m_unkUnused3;
            bool m_isOnGround2;
            double m_lastLandTime;
            float m_platformerVelocityRelated;
            bool m_maybeIsBoosted;
            double m_scaleXRelatedTime;
            bool m_decreaseBoostSlide;
            bool m_unkA29;
            bool m_isLocked;
            bool m_controlsDisabled;
            cocos2d::CCPoint m_lastGroundedPos;
            cocos2d::CCArray* m_touchingRings;
            gd::unordered_set<int> m_touchedRings;
            GameObject* m_lastActivatedPortal;
            bool m_hasEverJumped;
            bool m_hasEverHitRing;
            cocos2d::ccColor3B m_playerColor1;
            cocos2d::ccColor3B m_playerColor2;
            cocos2d::CCPoint m_position;
            bool m_isSecondPlayer;
            bool m_unkA99;
            double m_totalTime;
            bool m_isBeingSpawnedByDualPortal;
            float m_unkAAC;
            float m_unkAngle1;
            float m_yVelocityRelated3;
            bool m_defaultMiniIcon;
            bool m_swapColors;
            bool m_switchDashFireColor;
            int m_followRelated;
            gd::vector<float> m_playerFollowFloats;
            float m_unk838;
            int m_stateOnGround;
            unsigned char m_stateUnk;
            unsigned char m_stateNoStickX;
            unsigned char m_stateNoStickY;
            unsigned char m_stateUnk2;
            int m_stateBoostX;
            int m_stateBoostY;
            int m_maybeStateForce2;
            int m_stateScale;
            double m_platformerXVelocity;
            bool m_holdingRight;
            bool m_holdingLeft;
            bool m_leftPressedFirst;
            double m_scaleXRelated;
            bool m_maybeHasStopped;
            float m_xVelocityRelated;
            bool m_maybeGoingCorrectSlopeDirection;
            bool m_isSliding;
            double m_maybeSlopeForce;
            bool m_isOnIce;
            double m_physDeltaRelated;
            bool m_isOnGround4;
            int m_maybeSlidingTime;
            double m_maybeSlidingStartTime;
            double m_changedDirectionsTime;
            double m_slopeEndTime;
            bool m_isMoving;
            bool m_platformerMovingLeft;
            bool m_platformerMovingRight;
            bool m_isSlidingRight;
            double m_maybeChangedDirectionAngle;
            double m_unkUnused2;
            bool m_isPlatformer;
            int m_stateNoAutoJump;
            int m_stateDartSlide;
            int m_stateHitHead;
            int m_stateFlipGravity;
            float m_gravityMod;
            int m_stateForce;
            cocos2d::CCPoint m_stateForceVector;
            bool m_affectedByForces;
            gd::map<int, bool> m_jumpPadRelated;
            float m_somethingPlayerSpeedTime;
            float m_playerSpeedAC;
            bool m_fixRobotJump;
            gd::map<int, bool> m_holdingButtons;
            bool m_inputsLocked;
            gd::string m_currentRobotAnimation;
            bool m_gv0123;
            int m_iconRequestID;
            cocos2d::CCSpriteBatchNode* m_robotBatchNode;
            cocos2d::CCSpriteBatchNode* m_spiderBatchNode;
            cocos2d::CCArray* m_unk958;
            PlayerFireBoostSprite* m_robotFire;
            int m_unkUnused;
            GJBaseGameLayer* m_gameLayer;
            cocos2d::CCLayer* m_parentLayer;
            GJActionManager* m_actionManager;
            bool m_isOutOfBounds;
            float m_fallStartY;
            bool m_disablePlayerSqueeze;
            bool m_robotAnimation1Enabled;
            bool m_robotAnimation2Enabled;
            bool m_spiderAnimationEnabled;
            bool m_ignoreDamage;
            bool m_enable22Changes;
        };

        checkpoint_data from_playerobject(PlayerObject *po);
        void restore_playerobject(PlayerObject *po, checkpoint_data cpd);
    }
}
