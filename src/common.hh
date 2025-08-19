#pragma once

#include <chrono>

#include <Geode/Geode.hpp>

namespace uv {
    namespace gui {
        extern bool show;
        extern bool debug;
        extern std::chrono::time_point<std::chrono::steady_clock> toggle_time;
        
        void setup();
        void draw();
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
        
        frame_t get_frame(void);
        
        void reset(void);
        bool button(bool held, int button, bool player_1);
        void update_input(GJBaseGameLayer *self, float delta);
        void update_physics(GJBaseGameLayer *self, float delta);

        void clear();
        void save(std::string name);
        void load(std::string name);
    }

    namespace hacks {
        extern bool speedhack;
        extern float speedhack_multiplier;
        extern bool noclip, noclip_p1, noclip_p2;
        extern bool hitboxes;
        extern float hitboxes_thickness;
        extern bool layout_mode;
    }
    
    namespace practice_fix {
        // I am surprised this works too
        struct checkpoint_data {
            char data[sizeof(PlayerObject)];
            double rotation;
        };

        checkpoint_data from_playerobject(PlayerObject *po);
        void restore_playerobject(PlayerObject *po, checkpoint_data cpd);
    }
}
