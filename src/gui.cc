#include <imgui.h>

#include "common.hh"

namespace uv::gui {
    bool show = false;
    char macro_name[512];

    void setup() {}

    void draw() {
        if (!show) return;
        
        ImGui::Begin("uvBot");

        if (ImGui::BeginTabBar("uvBot TabBar")) {
            if (ImGui::BeginTabItem("Bot")) {
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::InputTextWithHint("##Macro Name", "Macro Name", macro_name, 512);

                if (ImGui::Button("Save", { ImGui::GetContentRegionAvail().x / 3, 0 })) uv::bot::save(macro_name);
                ImGui::SameLine();
                if (ImGui::Button("Load", { ImGui::GetContentRegionAvail().x / 2, 0 })) uv::bot::load(macro_name);
                ImGui::SameLine();
                if (ImGui::Button("Clear", { ImGui::GetContentRegionAvail().x, 0 })) uv::bot::clear();
                
                // I hate C++
                auto *state_pointer = reinterpret_cast<int*>(&uv::bot::current_state);
                
                ImGui::RadioButton("None", state_pointer, uv::bot::state::none);
                ImGui::SameLine();
                ImGui::RadioButton("Recording", state_pointer, uv::bot::state::recording);
                ImGui::SameLine();
                ImGui::RadioButton("Playing", state_pointer, uv::bot::state::playing);
 
                ImGui::Text("Input Actions: %d/%d", uv::bot::current_input_action, uv::bot::input_actions.size());
                ImGui::Text("Physic Actions:");
                ImGui::Bullet();
                ImGui::Text("Player 1: %d/%d", uv::bot::current_physic_player_1_action, uv::bot::physic_player_1_actions.size());
                ImGui::Bullet();
                ImGui::Text("Player 2: %d/%d", uv::bot::current_physic_player_2_action, uv::bot::physic_player_2_actions.size());
                ImGui::Text("Frame: %d", uv::bot::get_frame());

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Hacks")) {
                ImGui::Checkbox("##Speedhack Checkbox", &uv::hacks::speedhack);
                ImGui::SameLine();
                ImGui::DragFloat("##Speedhack", &uv::hacks::speedhack_multiplier, 0.01f, 0.0f, 2.0f, "Speedhack: %.2f");
                
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
        
        ImGui::End();
    }
}
