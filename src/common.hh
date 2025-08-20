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

        namespace recorder {
            struct options {
                int width, height;
                float fps, excess_render;
                std::string codec, bitrate;
                std::string output_path;
                std::string custom_options;
                bool hide_end_level_screen;
            };

            void start(options opt);
            void update(void);
            void end(void);

            extern bool recording;
            extern options recording_options;
        }
    }

    namespace hacks {
        extern bool speedhack;
        extern float speedhack_multiplier;
        extern bool noclip, noclip_p1, noclip_p2;
        extern bool hitboxes;
        extern float hitboxes_thickness;
        extern bool layout_mode;
        extern bool practice_fix;
    }
    
    namespace practice_fix {
        // Excuse my C-ness, I am a C engineer, not a C++ one
        struct checkpoint_data {
            double y_velocity;
            float rotation;
            cocos2d::CCPoint position;
        };

        checkpoint_data from_playerobject(PlayerObject *po);
        void restore_playerobject(PlayerObject *po, checkpoint_data cpd);
    }
}
