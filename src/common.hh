#pragma once

#include <chrono>

#include <Geode/Geode.hpp>

namespace uv {
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
        
        frame_t get_frame(void);
        
        void reset(void);
        bool button(bool held, int button, bool player_1);
        void update_input(GJBaseGameLayer *self, float delta);
        void update_physics(GJBaseGameLayer *self, float delta);

        void clear(void);
        void save(std::string name);
        void load(std::string name);

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
        extern bool speedhack;
        extern bool speedhack_classic;
        extern float speedhack_multiplier;
        extern bool lock_delta;
        extern bool noclip;
        extern bool noclip_p1;
        extern bool noclip_p2;
        extern bool hitboxes, hitboxes_trajectory;
        extern int hitboxes_trajectory_length;
        extern float hitboxes_thickness;
        extern std::vector<float> hitboxes_color_hazards, hitboxes_color_fill_hazards;
        extern std::vector<float> hitboxes_color_solids, hitboxes_color_fill_solids;
        extern std::vector<float> hitboxes_color_specials, hitboxes_color_fill_specials;
        extern std::vector<float> hitboxes_color_player, hitboxes_color_fill_player;
        extern bool layout_mode;
        extern bool practice_fix;
        extern bool copy_hack;

        void load(void);
        void save(void);

        namespace trajectory {
            extern bool making_trajectory, trajectory_end;
            
            void update(GJBaseGameLayer *gjbgl, PlayerObject *trajectory_players[2], cocos2d::CCDrawNode *trajectory_node);
        }
    }
    
    namespace practice_fix {
        struct checkpoint_data {
            double y_velocity;
            float rotation;
            cocos2d::CCPoint position;
        };

        checkpoint_data from_playerobject(PlayerObject *po);
        void restore_playerobject(PlayerObject *po, checkpoint_data cpd);
    }
}
