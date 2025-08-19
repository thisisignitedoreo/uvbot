#include <cstdio>

#include <Geode/Geode.hpp>

#include "common.hh"

namespace uv::bot {
    std::vector<input_action> input_actions;
    std::vector<physic_action> physic_player_1_actions;
    std::vector<physic_action> physic_player_2_actions;

    int current_input_action = 0;
    int current_physic_player_1_action = 0;
    int current_physic_player_2_action = 0;

    state current_state = state::none;

    bool permit_press = false;
    bool holding = false;

    static const auto macro_path = geode::Mod::get()->getSaveDir() / "Macros";

    const unsigned char macro_version = 1;
    
    frame_t get_frame(void) {
        PlayLayer *pl = PlayLayer::get();
        return pl ? static_cast<frame_t>(pl->m_gameState.m_currentProgress) : 0;
    }

    void reset(void) {
        if (current_state == state::recording) {
            frame_t frame = get_frame();
            std::erase_if(input_actions, [frame](input_action action) { return action.frame >= frame; });
            std::erase_if(physic_player_1_actions, [frame](physic_action action) { return action.frame >= frame; });
            std::erase_if(physic_player_2_actions, [frame](physic_action action) { return action.frame >= frame; });
        } else if (current_state == state::playing) {
            current_input_action = 0;
            current_physic_player_1_action = 0;
            current_physic_player_2_action = 0;
        }
    }

    bool button(bool held, int button, bool player_1) {
        if (current_state == state::recording) {
            if (!held && !holding) return true;
            holding = held;
            
            // Map button + player_1 to 0..5
            int action_button = button + ((1 - player_1) * 3 - 1);
            frame_t frame = get_frame();
            input_actions.push_back(input_action {
                frame,
                static_cast<unsigned char>(held + (action_button << 1))
            });
            
            return true;
        } else if (current_state == state::playing) {
            return permit_press;
        }
        return true;
    }
    
    void update_input(GJBaseGameLayer *self, float delta) {
        if (current_state == state::none) return;

        frame_t frame = get_frame();

        geode::log::debug("update_input(): frame = {}", frame);
        
        if (current_state == state::playing) {
            if (current_input_action < input_actions.size()) {
                input_action this_action = input_actions[current_input_action];
                if (this_action.frame == frame) {
                    int button = (this_action.flags >> 1) % 3 + 1;
                    permit_press = true;
                    self->handleButton(this_action.flags & 1, button, (this_action.flags >> 1) > 2);
                    permit_press = false;
                } else if (this_action.frame < frame) {
                    while (current_input_action < input_actions.size() && input_actions[current_input_action].frame < frame) {
                        current_input_action++;
                    }
                }
            }
        }
    }
    
    void update_physics(GJBaseGameLayer *self, float delta) {
        // Do not waste precious processing power
        if (current_state == state::none) return;

        frame_t frame = get_frame();

        geode::log::debug("update_physics(): frame = {}", frame);

        if (current_state == state::recording) {
            bool recorded_p1 = physic_player_1_actions.size() && physic_player_1_actions.back().frame >= frame;
            bool recorded_p2 = physic_player_2_actions.size() && physic_player_1_actions.back().frame >= frame;

            if (!recorded_p1) {
                physic_player_1_actions.push_back(physic_action {
                    frame,
                    self->m_player1->m_position.x,
                    self->m_player1->m_position.y,
                    self->m_player1->getRotation(),
                    self->m_player1->m_yVelocity,
                });
            }

            if (!recorded_p2 && self->m_gameState.m_isDualMode) {
                physic_player_2_actions.push_back(physic_action {
                    frame,
                    self->m_player2->m_position.x,
                    self->m_player2->m_position.y,
                    self->m_player2->getRotation(),
                    self->m_player2->m_yVelocity,
                });
            }
        } else if (current_state == state::playing) {
            if (current_physic_player_1_action < physic_player_1_actions.size()) {
                while (current_physic_player_1_action < physic_player_1_actions.size() && physic_player_1_actions[current_physic_player_1_action].frame < frame) {
                    current_physic_player_1_action++;
                }
                physic_action this_action = physic_player_1_actions[current_physic_player_1_action];
                if (this_action.frame == frame) {
                    self->m_player1->m_position.x = this_action.x;
                    self->m_player1->m_position.y = this_action.y;
                    self->m_player1->setRotation(this_action.rotation);
                    self->m_player1->m_yVelocity = this_action.y_velocity;
                }
            }
            
            if (current_physic_player_2_action < physic_player_2_actions.size()) {
                while (current_physic_player_2_action < physic_player_2_actions.size() && physic_player_2_actions[current_physic_player_2_action].frame < frame) {
                    current_physic_player_2_action++;
                }
                physic_action this_action = physic_player_2_actions[current_physic_player_2_action];
                if (this_action.frame == frame) {
                    self->m_player2->m_position.x = this_action.x;
                    self->m_player2->m_position.y = this_action.y;
                    self->m_player2->setRotation(this_action.rotation);
                    self->m_player2->m_yVelocity = this_action.y_velocity;
                }
            }
        }
    }

    void clear() {
        current_input_action = 0;
        current_physic_player_1_action = 0;
        current_physic_player_2_action = 0;

        input_actions.resize(0);
        physic_player_1_actions.resize(0);
        physic_player_1_actions.resize(0);
    }
    
    void save(std::string name) {
        std::ofstream file(macro_path / (name + ".uv"), std::ios::binary);

        file.write(reinterpret_cast<const char*>("UVBOT"), 5);
        file.write(reinterpret_cast<const char*>(&macro_version), 1);

        int input_action_amount = input_actions.size();
        int physic_player_1_action_amount = physic_player_1_actions.size();
        int physic_player_2_action_amount = physic_player_2_actions.size();

        file.write(reinterpret_cast<char*>(&input_action_amount), sizeof(int));
        file.write(reinterpret_cast<char*>(&physic_player_1_action_amount), sizeof(int));
        file.write(reinterpret_cast<char*>(&physic_player_2_action_amount), sizeof(int));

        for (auto &i : input_actions) {
            file.write(reinterpret_cast<char*>(&i.frame), sizeof(frame_t));
            file.write(reinterpret_cast<char*>(&i.flags), 1);
        }
        
        for (auto &i : physic_player_1_actions) {
            file.write(reinterpret_cast<char*>(&i.frame), sizeof(frame_t));
            file.write(reinterpret_cast<char*>(&i.x), sizeof(float));
            file.write(reinterpret_cast<char*>(&i.y), sizeof(float));
            file.write(reinterpret_cast<char*>(&i.rotation), sizeof(float));
            file.write(reinterpret_cast<char*>(&i.y_velocity), sizeof(double));
        }
        
        for (auto &i : physic_player_2_actions) {
            file.write(reinterpret_cast<char*>(&i.frame), sizeof(frame_t));
            file.write(reinterpret_cast<char*>(&i.x), sizeof(float));
            file.write(reinterpret_cast<char*>(&i.y), sizeof(float));
            file.write(reinterpret_cast<char*>(&i.rotation), sizeof(float));
            file.write(reinterpret_cast<char*>(&i.y_velocity), sizeof(double));
        }
        
        file.write(reinterpret_cast<const char*>("TOBVU"), 5);
    }
    
    void load(std::string name) {
        clear();
        
        std::ifstream file(macro_path / (name + ".uv"), std::ios::binary);

        char magic[5];
        file.read(reinterpret_cast<char*>(magic), 5);
        if (std::memcmp(magic, "UVBOT", 5) != 0) {
            geode::log::warn("Error loading macro file: Invalid starting magic sequence");
            return;
        }

        unsigned char version;
        file.read(reinterpret_cast<char*>(&version), 1);
        if (version != macro_version) {
            geode::log::warn("Error loading macro file: Version mismatch");
            return;
        }

        int input_action_amount = 0;
        int physic_player_1_action_amount = 0;
        int physic_player_2_action_amount = 0;

        file.read(reinterpret_cast<char*>(&input_action_amount), sizeof(int));
        file.read(reinterpret_cast<char*>(&physic_player_1_action_amount), sizeof(int));
        file.read(reinterpret_cast<char*>(&physic_player_2_action_amount), sizeof(int));

        for (int i = 0; i < input_action_amount; i++) {
            input_action action;
            file.read(reinterpret_cast<char*>(&action.frame), sizeof(frame_t));
            file.read(reinterpret_cast<char*>(&action.flags), 1);
            input_actions.push_back(action);
        }

        for (int i = 0; i < physic_player_1_action_amount; i++) {
            physic_action action;
            file.read(reinterpret_cast<char*>(&action.frame), sizeof(frame_t));
            file.read(reinterpret_cast<char*>(&action.x), sizeof(float));
            file.read(reinterpret_cast<char*>(&action.y), sizeof(float));
            file.read(reinterpret_cast<char*>(&action.rotation), sizeof(float));
            file.read(reinterpret_cast<char*>(&action.y_velocity), sizeof(double));
            physic_player_1_actions.push_back(action);
        }
        
        for (int i = 0; i < physic_player_2_action_amount; i++) {
            physic_action action;
            file.read(reinterpret_cast<char*>(&action.frame), sizeof(frame_t));
            file.read(reinterpret_cast<char*>(&action.x), sizeof(float));
            file.read(reinterpret_cast<char*>(&action.y), sizeof(float));
            file.read(reinterpret_cast<char*>(&action.rotation), sizeof(float));
            file.read(reinterpret_cast<char*>(&action.y_velocity), sizeof(double));
            physic_player_2_actions.push_back(action);
        }
        
        file.read(reinterpret_cast<char*>(magic), 5);
        if (std::memcmp(magic, "TOBVU", 5) != 0) {
            clear();
            geode::log::warn("Error loading macro file: Invalid ending magic sequence");
            return;
        }
    }
}
