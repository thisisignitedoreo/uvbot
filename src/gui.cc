#include <chrono>
#include <cmath>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <Geode/Geode.hpp>

#include <Geode/modify/CCEGLView.hpp>

#include "key_to_string.hh"

#include "common.hh"

namespace uv::gui {
    bool show = false;
    bool debug = false;
    std::chrono::steady_clock::time_point toggle_time;
    
    static int _last_key = 0;
    static int key = 0;
    static const char *last_id = nullptr;
    
    static bool show_demo = false, show_style_editor = false, pushed_colors = false, hitboxes_colors_opened = false, lm_colors_opened = false;
    
    static std::string macro_name, video_name, audio_name;
    
    static std::string merge_video_name, merge_audio_name, merge_output_name;
    static std::string merge_video_path, merge_audio_path, merge_output_path;
    static std::string merge_arguments = "-map 0:v:0 -map 1:a:0 -c:v copy -c:a copy -shortest";
    static std::string raw_arguments = "-i \"{showcases}/file.mp4\" \"{showcases}/file.mkv\"";
    
    static const std::chrono::steady_clock::duration animation_duration = std::chrono::milliseconds(150);

    static bool recording = false, audio_recording = false;
    static bool record_audio = false, merge_audio = false;

    uv::recorder::options render_opts = {
        .width = 1920,
        .height = 1080,
        .fps = 60.0f,
        .excess_render = 3.0f,
        .codec = "libx264",
        .bitrate = "50M",
        .output_path = geode::utils::string::pathToString(uv::showcase_path / ".mp4"),
        .custom_options = "-pix_fmt yuv420p -vf \"vflip\"",
        .hide_end_level_screen = true,
        .fade_out = false,
    };
    
    uv::recorder::audio::options audio_opts = {
        .output_path = geode::utils::string::pathToString(uv::showcase_path / ".wav"),
        .music_volume = 1.0f,
        .sfx_volume = 1.0f,
        .excess_render = 3.0f,
   };
    
    static const std::filesystem::path ffmpeg_path = geode::dirs::getGameDir() / "ffmpeg.exe";

    void setup(void) {
        ImGuiIO &io = ImGui::GetIO();
        ImGuiStyle &style = ImGui::GetStyle();

        // OpenSans font
        // https://fonts.google.com/specimen/Open+Sans
        
        io.Fonts->AddFontFromFileTTF(geode::utils::string::pathToString(geode::Mod::get()->getResourcesDir() / "OpenSans-Regular.ttf").c_str(), 22.0f);
        
        // Windark style by DestroyerDarkNess from ImThemes
        // And some small changes
        
        style.Alpha = 1.0f;
        style.DisabledAlpha = 0.6f;
        style.WindowPadding = ImVec2(12.0f, 8.0f);
        style.WindowRounding = 8.4f;
        style.WindowBorderSize = 1.0f;
        style.WindowMinSize = ImVec2(100.0f, 100.0f);
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
        style.WindowMenuButtonPosition = ImGuiDir_None;
        style.ChildRounding = 3.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupRounding = 3.0f;
        style.PopupBorderSize = 1.0f;
        style.FramePadding = ImVec2(8.0f, 3.0f);
        style.FrameRounding = 3.0f;
        style.FrameBorderSize = 1.0f;
        style.ItemSpacing = ImVec2(8.0f, 4.0f);
        style.ItemInnerSpacing = ImVec2(8.0f, 4.0f);
        style.CellPadding = ImVec2(4.0f, 2.0f);
        style.IndentSpacing = 21.0f;
        style.ColumnsMinSpacing = 6.0f;
        style.ScrollbarSize = 5.6f;
        style.ScrollbarRounding = 18.0f;
        style.SeparatorTextBorderSize = 1.0f;
        style.SeparatorTextAlign = ImVec2(0.5f, 0.5f);
        style.GrabMinSize = 10.0f;
        style.GrabRounding = 3.0f;
        style.TabRounding = 3.0f;
        style.TabBorderSize = 1.0f;
        style.ColorButtonPosition = ImGuiDir_Right;
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
        style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
        
        style.Colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        style.Colors[ImGuiCol_WindowBg]               = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
        style.Colors[ImGuiCol_ChildBg]                = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
        style.Colors[ImGuiCol_PopupBg]                = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
        style.Colors[ImGuiCol_Border]                 = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        style.Colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_FrameBg]                = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
        style.Colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
        style.Colors[ImGuiCol_FrameBgActive]          = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        style.Colors[ImGuiCol_TitleBg]                = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
        style.Colors[ImGuiCol_TitleBgActive]          = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
        style.Colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
        style.Colors[ImGuiCol_MenuBarBg]              = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
        style.Colors[ImGuiCol_CheckMark]              = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
        style.Colors[ImGuiCol_SliderGrab]             = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
        style.Colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        style.Colors[ImGuiCol_Button]                 = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
        style.Colors[ImGuiCol_ButtonHovered]          = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
        style.Colors[ImGuiCol_ButtonActive]           = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        style.Colors[ImGuiCol_Header]                 = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
        style.Colors[ImGuiCol_HeaderHovered]          = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        style.Colors[ImGuiCol_HeaderActive]           = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        style.Colors[ImGuiCol_Separator]              = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
        style.Colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        style.Colors[ImGuiCol_SeparatorActive]        = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        style.Colors[ImGuiCol_ResizeGrip]             = ImVec4(0.22f, 0.22f, 0.22f, 0.00f);
        style.Colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.25f, 0.25f, 0.25f, 0.00f);
        style.Colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.30f, 0.30f, 0.30f, 0.00f);
        style.Colors[ImGuiCol_TabHovered]             = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
        style.Colors[ImGuiCol_Tab]                    = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
        style.Colors[ImGuiCol_TabSelected]            = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        style.Colors[ImGuiCol_TabSelectedOverline]    = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_TabDimmed]              = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
        style.Colors[ImGuiCol_TabDimmedSelected]      = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
        style.Colors[ImGuiCol_TabDimmedSelectedOverline]  = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
        style.Colors[ImGuiCol_PlotLines]              = ImVec4(0.00f, 0.47f, 0.84f, 1.00f);
        style.Colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.00f, 0.33f, 0.60f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogram]          = ImVec4(0.00f, 0.47f, 0.84f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(0.00f, 0.33f, 0.60f, 1.00f);
        style.Colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        style.Colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
        style.Colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
        style.Colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        style.Colors[ImGuiCol_TextLink]               = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
        style.Colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        style.Colors[ImGuiCol_NavCursor]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        style.Colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        style.Colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    }
    
    static void keycode_selector(const char *id, int d) {
        bool selected = last_id && !std::strcmp(id, last_id);

        if (ImGui::Button(selected ? "..." : KeyCodeToStringSwitch(uv::hacks::get(id, d)), { ImGui::CalcTextSize("KeypadMultiply").x, 0 })) {
            key = 0;
            last_id = id;
        }

        if (selected && key != 0) {
            last_id = nullptr;
            uv::hacks::set(id, key);
        }
    }
    
    static void draw_main_ui(void) {
        if (ImGui::BeginTabBar("uvBot TabBar")) {
            if (ImGui::BeginTabItem("Bot")) {
                // I hate C++
                auto *state_pointer = reinterpret_cast<int*>(&uv::bot::current_state);
                
                bool changed_none = ImGui::RadioButton("None", state_pointer, uv::bot::state::none);
                ImGui::SameLine();
                bool changed_recording = ImGui::RadioButton("Recording", state_pointer, uv::bot::state::recording);
                ImGui::SameLine();
                bool changed_playing = ImGui::RadioButton("Playing", state_pointer, uv::bot::state::playing);

                ImGui::Spacing();

                bool changed = changed_none || changed_recording || changed_playing;
                
                if (changed) {
                    uv::bot::current_input_action = 0;
                    uv::bot::current_physic_player_1_action = 0;
                    uv::bot::current_physic_player_2_action = 0;
                }

                if (changed_recording) uv::bot::record_tps = uv::hacks::get("tps", 240.0f);
                
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::InputTextWithHint("##Macro Name", "Macro Name", &macro_name);

                float button_widths = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2) / 3;
                
                ImGui::BeginDisabled(macro_name.empty());
                if (ImGui::Button("Save", { button_widths, 0 })) uv::bot::save(geode::utils::string::trim(macro_name));
                if (macro_name.empty() && ImGui::BeginItemTooltip()) {
                    ImGui::Text("To save a macro, input its name");
                    ImGui::EndTooltip();
                }
                ImGui::SameLine();
                if (ImGui::Button("Load", { button_widths, 0 })) uv::bot::load(geode::utils::string::trim(macro_name)); 
                if (macro_name.empty() && ImGui::BeginItemTooltip()) {
                    ImGui::Text("To load a macro, input its name");
                    ImGui::EndTooltip();
                }
                ImGui::EndDisabled();
                ImGui::SameLine();
                if (ImGui::Button("Clear", { button_widths, 0 })) {
                    uv::bot::clear();
                    macro_name.clear();
                }
                
                ImGui::Spacing();
                
                if (ImGui::Button("Open Macros folder", { ImGui::GetContentRegionAvail().x, 0 })) geode::utils::file::openFolder(uv::macro_path);

                ImGui::SeparatorText("Macros");

                bool not_empty = false;
                std::error_code error;
                uv::bot::foreign::error load_error = uv::bot::foreign::error::none;
                for (auto const &dir_entry : std::filesystem::directory_iterator(uv::macro_path, error)) {
                    not_empty = true; // I am too lazy to do this the right way
                    std::string dir_str = geode::utils::string::pathToString(dir_entry.path());
                    if (dir_entry.is_regular_file()) {
                        if (dir_str.ends_with(".uv")) {
                            std::string path = geode::utils::string::pathToString(dir_entry.path());
                            int from = path.rfind(dir_entry.path().preferred_separator);
                            if (from == std::string::npos) from = 0;
                            std::string name = path.substr(from + 1, path.size() - from - 4);
                            if (ImGui::Button(name.c_str(), { ImGui::GetContentRegionAvail().x, 0 })) {
                                uv::bot::load(name);
                                macro_name = name;
                            }
                        } else {
                            for (auto &i : uv::bot::foreign::supported_exts) {
                                if (dir_str.ends_with(i)) {
                                    std::string path = geode::utils::string::pathToString(dir_entry.path());
                                    int from = path.rfind(dir_entry.path().preferred_separator);
                                    if (from == std::string::npos) from = 0;
                                    std::string name = path.substr(from + 1, path.size() - from - 1);
                                    if (ImGui::Button(name.c_str(), { ImGui::GetContentRegionAvail().x, 0 })) {
                                        load_error = uv::bot::foreign::load(name);
                                        macro_name = name.substr(0, name.rfind("."));
                                        if (!uv::hacks::set<bool>("shown-foriegn-macro-warn", true)) ImGui::OpenPopup("Hey!##Foreign Macro Popup");
                                    }
                                }
                            }
                        }
                    }
                }

                if (!not_empty) ImGui::TextDisabled("No macros saved... :_(");

                if (error) geode::log::debug("Error reading macro directory: {}", error.message());

                if (load_error == uv::bot::foreign::error::unsupported_slc_v2) ImGui::OpenPopup("Unsupported!##SLC v2");
                if (load_error == uv::bot::foreign::error::unsupported_slc_v3) ImGui::OpenPopup("Unsupported!##SLC v3");
                if (load_error == uv::bot::foreign::error::unsupported) ImGui::OpenPopup("Unsupported!");

                if (ImGui::BeginPopupModal("Hey!##Foreign Macro Popup")) {
                    ImGui::Text("You are loading a foreign macro, meaning the macro is made by and for another bot.\n"
                    "This macro is not guaranteed to play as well as the original, and/or be accurate.\n"
                    "Think of this feature as a built in macro converter.");
                    ImGui::Dummy(ImVec2(0, ImGui::GetWindowSize().y - ImGui::GetCursorPosY() - ImGui::GetTextLineHeight() - ImGui::GetStyle().FramePadding.y * 2.0f - ImGui::GetStyle().WindowPadding.y * 2));
                    if (ImGui::Button("Okay, got it!", { ImGui::GetContentRegionAvail().x, 0 })) ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }

                if (ImGui::BeginPopupModal("Unsupported!##SLC v2")) {
                    ImGui::Text("This macro is SLC v2, and it is not currently a supported format.\nIt is planned however - you'll have to wait for version v0.2.1");
                    ImGui::Dummy(ImVec2(0, ImGui::GetWindowSize().y - ImGui::GetCursorPosY() - ImGui::GetTextLineHeight() - ImGui::GetStyle().FramePadding.y * 2.0f - ImGui::GetStyle().WindowPadding.y * 2));
                    if (ImGui::Button("Okay, got it!", { ImGui::GetContentRegionAvail().x, 0 })) ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }

                if (ImGui::BeginPopupModal("Unsupported!##SLC v3")) {
                    ImGui::Text("This macro is SLC v3, and it is not currently a supported format.\nIt is planned however - you'll have to wait for version v0.2.1");
                    ImGui::Dummy(ImVec2(0, ImGui::GetWindowSize().y - ImGui::GetCursorPosY() - ImGui::GetTextLineHeight() - ImGui::GetStyle().FramePadding.y * 2.0f - ImGui::GetStyle().WindowPadding.y * 2));
                    if (ImGui::Button("Okay, got it!", { ImGui::GetContentRegionAvail().x, 0 })) ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }

                if (ImGui::BeginPopupModal("Unsupported!")) {
                    ImGui::Text("This macro is unsupported. No further information.");
                    ImGui::Dummy(ImVec2(0, ImGui::GetWindowSize().y - ImGui::GetCursorPosY() - ImGui::GetTextLineHeight() - ImGui::GetStyle().FramePadding.y * 2.0f - ImGui::GetStyle().WindowPadding.y * 2));
                    if (ImGui::Button("Okay, got it!", { ImGui::GetContentRegionAvail().x, 0 })) ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }
                
                ImGui::EndTabItem();
            }

            #define checkbox_id(n, i, d) do { \
                bool aa = uv::hacks::get((i), d); \
                ImGui::Checkbox((n), &aa); \
                uv::hacks::set((i), aa); \
            } while (0)
            
            if (ImGui::BeginTabItem("Hacks")) {
                ImGui::SeparatorText("Botting");

                ImGui::BeginDisabled(uv::bot::current_state != uv::bot::none);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                
                float tps = uv::hacks::get("tps", 240.0f);
                ImGui::DragFloat("##TPS", &tps, 0.1f, 240.0f, 10000.0f, "TPS: %.2f", ImGuiSliderFlags_AlwaysClamp);
                uv::hacks::set<float>("tps", 240.0f);
                
                ImGui::EndDisabled();

                checkbox_id("##Speedhack Checkbox", "speedhack", false);
                
                ImGui::SameLine();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                
                float speedhack_multiplier = uv::hacks::get("speedhack-multiplier", 1.0f);
                ImGui::DragFloat("##Speedhack", &speedhack_multiplier, 0.01f, 0.0f, 3.0f, "Speedhack: %.2fx");
                if (speedhack_multiplier <= 0.0f) speedhack_multiplier = 1.0f;
                uv::hacks::set("speedhack-multiplier", speedhack_multiplier);

                checkbox_id("Classic Mode Speedhack", "speedhack-classic", false);
                
                ImGui::SameLine();
                ImGui::TextDisabled("?");
                ImGui::SetItemTooltip("Instead of making the game smoothly slower, classic mode slows down the framerate.\nUseful for recording the game with OBS or something.");

                checkbox_id("Lock DeltaTime", "lock-delta", false);
                
                ImGui::SameLine();
                ImGui::TextDisabled("?");
                ImGui::SetItemTooltip("Disables frame skip when the game can't keep up with the target FPS.\nUseful for showcasing.");
                
                checkbox_id("Practice Fix", "practice-fix", true);
                
                checkbox_id("Accuracy Fix", "accuracy-fix", true);

                ImGui::SeparatorText("Player");
                
                bool noclip = uv::hacks::get("noclip", false);
                bool noclip_p1 = uv::hacks::get("noclip-p1", false);
                bool noclip_p2 = uv::hacks::get("noclip-p2", false);

                if (ImGui::Checkbox("Noclip", &noclip) && noclip) {
                    noclip_p1 = false;
                    noclip_p2 = false;
                }
                ImGui::SameLine();
                if (ImGui::Checkbox("Player 1", &noclip_p1) && noclip_p1) {
                    noclip = false;
                    noclip_p2 = false;
                }
                ImGui::SameLine();
                if (ImGui::Checkbox("Player 2", &noclip_p2) && noclip_p2) {
                    noclip_p1 = false;
                    noclip = false;
                }
                
                uv::hacks::set("noclip", noclip);
                uv::hacks::set("noclip-p1", noclip_p1);
                uv::hacks::set("noclip-p2", noclip_p2);
                
                ImGui::SeparatorText("Hitboxes");
                
                checkbox_id("Show Hitboxes", "hitboxes", false);
                ImGui::SameLine();
                checkbox_id("Show Trajectory", "hitboxes-trajectory", false);

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                
                float hitboxes_thickness = uv::hacks::get("hitboxes-thickness", 0.3f);
                ImGui::DragFloat("##Hitboxes Thickness", &hitboxes_thickness, 0.01f, 0.01f, 3.0f, "Hitboxes Thickness: %.2f", ImGuiSliderFlags_AlwaysClamp);
                uv::hacks::set("hiboxes-thickness", hitboxes_thickness);

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                
                int hitboxes_trajectory_length = uv::hacks::get("hitboxes-trajectory-length", 100);
                ImGui::DragInt("##Hitboxes Trajectory Length", &hitboxes_trajectory_length, 1, 1, 1000, "Trajectory Length: %d frames");
                uv::hacks::set("hitboxes-trajectory-length", hitboxes_trajectory_length);
                
                if (ImGui::ArrowButton("Hitboxes Colors Collapser", hitboxes_colors_opened ? ImGuiDir_Down : ImGuiDir_Right)) hitboxes_colors_opened = !hitboxes_colors_opened;
                ImGui::SameLine();
                ImGui::Text("Hitboxes Colors");
                
                if (hitboxes_colors_opened) {
                    std::vector<float> hitboxes_color_hazards = uv::hacks::get<std::vector<float>>("hitboxes-color-hazards", { 1.0f, 0.0f, 0.0f, 1.0f });
                    std::vector<float> hitboxes_color_fill_hazards = uv::hacks::get<std::vector<float>>("hitboxes-color-fill-hazards", { 1.0f, 0.0f, 0.0f, 0.2f });
                    std::vector<float> hitboxes_color_solids = uv::hacks::get<std::vector<float>>("hitboxes-color-solids", { 0.0f, 0.0f, 1.0f, 1.0f });
                    std::vector<float> hitboxes_color_fill_solids = uv::hacks::get<std::vector<float>>("hitboxes-color-fill-solids", { 0.0f, 0.0f, 1.0f, 0.2f });
                    std::vector<float> hitboxes_color_specials = uv::hacks::get<std::vector<float>>("hitboxes-color-specials", { 0.0f, 1.0f, 0.0f, 1.0f });
                    std::vector<float> hitboxes_color_fill_specials = uv::hacks::get<std::vector<float>>("hitboxes-color-fill-specials", { 0.0f, 1.0f, 0.0f, 0.2f });
                    std::vector<float> hitboxes_color_player = uv::hacks::get<std::vector<float>>("hitboxes-color-player", { 1.0f, 1.0f, 0.0f, 1.0f });
                    std::vector<float> hitboxes_color_fill_player = uv::hacks::get<std::vector<float>>("hitboxes-color-fill-player", { 1.0f, 1.0f, 0.0f, 0.2f });
                    
                    ImGui::ColorEdit4("Hazards", hitboxes_color_hazards.data());
                    ImGui::ColorEdit4("Solids", hitboxes_color_solids.data());
                    ImGui::ColorEdit4("Specials", hitboxes_color_specials.data());
                    ImGui::ColorEdit4("Player", hitboxes_color_player.data());
                    ImGui::Spacing();
                    ImGui::ColorEdit4("Hazards Fill", hitboxes_color_fill_hazards.data());
                    ImGui::ColorEdit4("Solids Fill", hitboxes_color_fill_solids.data());
                    ImGui::ColorEdit4("Specials Fill", hitboxes_color_fill_specials.data());
                    ImGui::ColorEdit4("Player Fill", hitboxes_color_fill_player.data());
                    
                    uv::hacks::set<std::vector<float>>("hitboxes-color-hazards", hitboxes_color_hazards);
                    uv::hacks::set<std::vector<float>>("hitboxes-color-fill-hazards", hitboxes_color_fill_hazards);
                    uv::hacks::set<std::vector<float>>("hitboxes-color-solids", hitboxes_color_solids);
                    uv::hacks::set<std::vector<float>>("hitboxes-color-fill-solids", hitboxes_color_fill_solids);
                    uv::hacks::set<std::vector<float>>("hitboxes-color-specials", hitboxes_color_specials);
                    uv::hacks::set<std::vector<float>>("hitboxes-color-fill-specials", hitboxes_color_fill_specials);
                    uv::hacks::set<std::vector<float>>("hitboxes-color-player", hitboxes_color_player);
                    uv::hacks::set<std::vector<float>>("hitboxes-color-fill-player", hitboxes_color_fill_player);
                }
                
                ImGui::SeparatorText("Layout Mode");
                checkbox_id("Layout Mode", "layout-mode", false);
                checkbox_id("Remove Shake Triggers", "layout-mode-shake", true);
                ImGui::Spacing();
                // checkbox_id("Show Triggers", "show-triggers", false);
                // ImGui::Spacing();
                if (ImGui::ArrowButton("Layout Mode Colors Opener", lm_colors_opened ? ImGuiDir_Down : ImGuiDir_Right)) lm_colors_opened = !lm_colors_opened;
                ImGui::SameLine();
                ImGui::Text("Layout Mode Colors");
                
                if (lm_colors_opened) {
                    std::vector<float> bg = uv::hacks::get<std::vector<float>>("layout-mode-bg-color", { 0.0f, 0.0f, 0.0f });
                    std::vector<float> g  = uv::hacks::get<std::vector<float>>("layout-mode-ground-color", { 0.0f, 0.0f, 0.0f });
                    std::vector<float> l  = uv::hacks::get<std::vector<float>>("layout-mode-line-color", { 1.0f, 1.0f, 1.0f });
                    std::vector<float> mg = uv::hacks::get<std::vector<float>>("layout-mode-mg-color", { 0.0f, 0.0f, 0.0f });
                    std::vector<float> ee = uv::hacks::get<std::vector<float>>("layout-mode-ee-color", { 1.0f, 1.0f, 1.0f });
                    
                    ImGui::ColorEdit3("BG", bg.data());
                    ImGui::ColorEdit3("Ground", g.data());
                    ImGui::ColorEdit3("Line", l.data());
                    ImGui::ColorEdit3("MG", mg.data());
                    ImGui::ColorEdit3("Everything else", ee.data());
                    
                    uv::hacks::set<std::vector<float>>("layout-mode-bg-color", bg);
                    uv::hacks::set<std::vector<float>>("layout-mode-ground-color", g);
                    uv::hacks::set<std::vector<float>>("layout-mode-line-color", l);
                    uv::hacks::set<std::vector<float>>("layout-mode-mg-color", mg);
                    uv::hacks::set<std::vector<float>>("layout-mode-ee-color", ee);
                }
                
                ImGui::SeparatorText("Other");
                checkbox_id("Copy Hack", "copy-hack", false);
                
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Recorder")) {
                std::error_code error;
                if (!std::filesystem::is_regular_file(ffmpeg_path, error) || !std::filesystem::exists(ffmpeg_path, error)) {
                    ImGui::PushTextWrapPos(0.0f);
                    ImGui::Text("For the internal recorder feature to work, you need to copy ffmpeg.exe to Geometry Dash installation path.");
                    
                    ImGui::Dummy(ImVec2(0, ImGui::GetWindowSize().y - ImGui::GetCursorPosY() - ImGui::GetTextLineHeight() - ImGui::GetStyle().FramePadding.y * 2.0f - ImGui::GetStyle().WindowPadding.y * 2));
                    if (ImGui::Button("Open GD folder", { ImGui::GetContentRegionAvail().x, 0 })) geode::utils::file::openFolder(geode::dirs::getGameDir());
                    ImGui::PopTextWrapPos();
                } else {
                    if (ImGui::BeginTabBar("uvBot Recorder Sub-TabBar")) {
                        if (ImGui::BeginTabItem("Video")) {
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            if (ImGui::InputTextWithHint("##Video Filename", "Video Filename (e.g. file.mp4)", &video_name)) {
                                render_opts.output_path = geode::utils::string::pathToString(uv::showcase_path / video_name);
                            }

                            // This is probably too overengineered but I don't care

                            float space_without_text = (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("x").x - ImGui::CalcTextSize("@").x - ImGui::GetStyle().ItemSpacing.x * 4) / 3;
                            float integ;
                            float fract = std::modf(space_without_text, &integ);
                            
                            ImGui::SetNextItemWidth(integ);
                            ImGui::DragInt("##Width", &render_opts.width, 1, 1, 9999, "%upx");
                            ImGui::SameLine(); ImGui::Text("x"); ImGui::SameLine();
                            ImGui::SetNextItemWidth(fract >= 0.666f ? integ + 1 : integ);
                            ImGui::DragInt("##Height", &render_opts.height, 1, 1, 9999, "%upx");
                            ImGui::SameLine(); ImGui::Text("@"); ImGui::SameLine();
                            ImGui::SetNextItemWidth(fract >= 0.333f ? integ + 1 : integ);
                            ImGui::DragFloat("##FPS", &render_opts.fps, 1.0f, 1.0f, 9999.9f, "%.2f FPS");

                            ImGui::Spacing();

                            ImGui::PushItemWidth(-ImGui::CalcTextSize("Custom Options?").x - ImGui::GetStyle().WindowPadding.x - ImGui::GetStyle().ItemSpacing.x); // The longest of them all

                            ImGui::InputText("Bitrate", &render_opts.bitrate);
                            ImGui::SameLine();
                            ImGui::TextDisabled("?");
                            ImGui::SetItemTooltip(
                                "Bitrate controls video quality and file size.\n"
                                "Higher bitrate = better quality, but larger files.\n"
                                "'M' means megabits per second (e.g. 50M ~ 50 Mbps)."
                            );
                            ImGui::InputText("Video Codec", &render_opts.codec);
                            ImGui::SameLine();
                            ImGui::TextDisabled("?");
                            ImGui::SetItemTooltip(
                                "Codec used for video compression.\n"
                                "Different codecs vary in quality, compatibility, and performance.\n"
                                "Some codecs (e.g. NVENC, VAAPI) can use the GPU for faster rendering."
                            );
                            ImGui::InputText("Custom Options", &render_opts.custom_options);
                            ImGui::SameLine();
                            ImGui::TextDisabled("?");
                            ImGui::SetItemTooltip(
                                "Extra options passed directly to FFmpeg.\n"
                                "Useful for advanced users to apply filters or effects.\n"
                                "If unsure, leave the default value."
                            );

                            ImGui::PopItemWidth();

                            ImGui::Spacing();

                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            ImGui::DragFloat("##Excess render", &render_opts.excess_render, 0.1f, 0.0f, 5.0f, "Render after level ends: %.1fs");

                            ImGui::Checkbox("Hide End Level menu", &render_opts.hide_end_level_screen);

                            ImGui::Checkbox("Fade Out", &render_opts.fade_out);
                            ImGui::SameLine();
                            ImGui::TextDisabled("?");
                            ImGui::SetItemTooltip(
                                "Fades out only the extra rendering beyond the level.\n"
                                "This is difficult to achieve with FFmpeg filters,\n"
                                "so it is handled using an in-game overlay."
                            );
                            
                            ImGui::Spacing();
                            
                            if (ImGui::Button("Open Showcases folder", { ImGui::GetContentRegionAvail().x, 0 })) geode::utils::file::openFolder(uv::showcase_path);

                            space_without_text = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 3) / 4;

                            // Thanks toby for bitrate values
                            
                            ImGui::SeparatorText("Presets");
                            
                            if (ImGui::Button("720p", { space_without_text, 0 })) { render_opts.bitrate = "25M"; render_opts.width = 1280; render_opts.height = 720; }
                            ImGui::SameLine();
                            if (ImGui::Button("1080p", { space_without_text, 0 })) { render_opts.bitrate = "50M"; render_opts.width = 1920; render_opts.height = 1080; }
                            ImGui::SameLine();
                            if (ImGui::Button("2K", { space_without_text, 0 })) { render_opts.bitrate = "70M"; render_opts.width = 2560; render_opts.height = 1440; }
                            ImGui::SameLine();
                            if (ImGui::Button("4K", { space_without_text, 0 })) { render_opts.bitrate = "80M"; render_opts.width = 3840; render_opts.height = 2160; }
                            
                            if (ImGui::Button("30 FPS", { space_without_text, 0 })) render_opts.fps = 30.0f;
                            ImGui::SameLine();
                            if (ImGui::Button("60 FPS", { space_without_text, 0 })) render_opts.fps = 60.0f;
                            ImGui::SameLine();
                            if (ImGui::Button("90 FPS", { space_without_text, 0 })) render_opts.fps = 90.0f;
                            ImGui::SameLine();
                            if (ImGui::Button("120 FPS", { space_without_text, 0 })) render_opts.fps = 120.0f;
                            
                            ImGui::Dummy(ImVec2(0, ImGui::GetWindowSize().y - ImGui::GetCursorPosY() - ImGui::GetTextLineHeight() - ImGui::GetStyle().FramePadding.y * 2.0f - ImGui::GetStyle().WindowPadding.y * 2));

                            bool disabled = video_name.empty();
                            ImGui::BeginDisabled(disabled);
                            
                            recording = uv::recorder::recording;
                            if (ImGui::Button(recording ? "Stop Recording" : "Start Recording", { ImGui::GetContentRegionAvail().x, 0 })) {
                                recording = !recording;
                                if (recording) {
                                    uv::recorder::start(render_opts);
                                } else {
                                    uv::recorder::end();
                                }
                            }
                            
                            if (disabled && ImGui::BeginItemTooltip()) {
                                if (video_name.empty()) ImGui::Text("Input the video filename");
                                ImGui::EndTooltip();
                            }

                            ImGui::EndDisabled();

                            ImGui::EndTabItem();
                        }

                        if (ImGui::BeginTabItem("Audio")) {
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            if (ImGui::InputTextWithHint("##Audio Filename", "Audio Filename (e.g. file.wav)", &audio_name)) {
                                audio_opts.output_path = geode::utils::string::pathToString(uv::showcase_path / audio_name);
                            }

                            ImGui::Spacing();

                            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
                            ImGui::DragFloat("##Music volume", &audio_opts.music_volume, 0.01f, 0.0f, 1.0f, "Music volume: %.2f");
                            ImGui::DragFloat("##SFX volume", &audio_opts.sfx_volume, 0.01f, 0.0f, 1.0f, "SFX volume: %.2f");
                            ImGui::DragFloat("##Excess render", &audio_opts.excess_render, 0.1f, 0.0f, 5.0f, "Render after level ends: %.1fs");
                            ImGui::PopItemWidth();
                            
                            ImGui::Spacing();
                            
                            if (ImGui::Button("Open Showcases folder", { ImGui::GetContentRegionAvail().x, 0 })) geode::utils::file::openFolder(uv::showcase_path);

                            ImGui::Dummy(ImVec2(0, ImGui::GetWindowSize().y - ImGui::GetCursorPosY() - ImGui::GetTextLineHeight() - ImGui::GetStyle().FramePadding.y * 2.0f - ImGui::GetStyle().WindowPadding.y * 2));
                            
                            bool disabled = audio_name.empty() || !audio_name.ends_with(".wav");
                            ImGui::BeginDisabled(disabled);
                            
                            audio_recording = uv::recorder::audio::recording;
                            if (ImGui::Button(audio_recording ? "Stop Audio Recording" : "Start Audio Recording", { ImGui::GetContentRegionAvail().x, 0 })) {
                                audio_recording = !audio_recording;
                                if (audio_recording) {
                                    uv::recorder::audio::init(audio_opts);
                                } else {
                                    uv::recorder::audio::end();
                                }
                            }
                            
                            if (disabled && ImGui::BeginItemTooltip()) {
                                if (audio_name.empty()) ImGui::Text("Input the audio filename");
                                else if (!audio_name.ends_with(".wav")) ImGui::Text("Only .wav format is supported right now");
                                ImGui::EndTooltip();
                            }

                            ImGui::EndDisabled();

                            ImGui::EndTabItem();
                        }

                        if (ImGui::BeginTabItem("Merge")) {
                            ImGui::BeginDisabled(!uv::recorder::process_done());
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            if (ImGui::InputTextWithHint("##Merge Video Filename", "Video Filename (e.g. file.mp4)", &merge_video_name)) {
                                merge_video_path = geode::utils::string::pathToString(uv::showcase_path / merge_video_name);
                            }
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            if (ImGui::InputTextWithHint("##Merge Audio Filename", "Audio Filename (e.g. file.wav)", &merge_audio_name)) {
                                merge_audio_path = geode::utils::string::pathToString(uv::showcase_path / merge_audio_name);
                            }
                            
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            if (ImGui::InputTextWithHint("##Merge Output Filename", "Output Filename (e.g. file +music.mp4)", &merge_output_name)) {
                                merge_output_path = geode::utils::string::pathToString(uv::showcase_path / merge_output_name);
                            }
                            
                            ImGui::Spacing();

                            ImGui::SetNextItemWidth(-ImGui::CalcTextSize("Arguments").x - ImGui::GetStyle().WindowPadding.x);
                            ImGui::InputText("Arguments", &merge_arguments);
                           
                            ImGui::Spacing();
                            
                            if (ImGui::Button("Open Showcases folder", { ImGui::GetContentRegionAvail().x, 0 })) geode::utils::file::openFolder(uv::showcase_path);

                            ImGui::Dummy(ImVec2(0, ImGui::GetWindowSize().y - ImGui::GetCursorPosY() - ImGui::GetTextLineHeight() - ImGui::GetStyle().FramePadding.y * 2.0f - ImGui::GetStyle().WindowPadding.y * 2));

                            std::error_code error;
                            bool disabled = merge_video_name.empty() || merge_audio_name.empty();
                            disabled = disabled || !std::filesystem::exists(merge_video_path, error) || !std::filesystem::exists(merge_audio_path, error);
                            disabled = disabled || !std::filesystem::is_regular_file(merge_video_path, error) || !std::filesystem::is_regular_file(merge_audio_path, error);

                            ImGui::BeginDisabled(disabled);
                            
                            if (ImGui::Button("Merge", { ImGui::GetContentRegionAvail().x, 0 })) uv::recorder::merge(merge_video_path, merge_audio_path, merge_output_path, merge_arguments);
                            
                            if (disabled && ImGui::BeginItemTooltip()) {
                                if (merge_video_name.empty()) ImGui::Text("Input the video filename");
                                else if (!std::filesystem::exists(merge_video_path, error)) ImGui::Text("No video with that filename");
                                else if (!std::filesystem::is_regular_file(merge_video_path, error)) ImGui::Text("Video with that filename is not a file");
                                if (merge_audio_name.empty()) ImGui::Text("Input the audio filename");
                                else if (!std::filesystem::exists(merge_audio_path, error)) ImGui::Text("No audio with that filename");
                                else if (!std::filesystem::is_regular_file(merge_audio_path, error)) ImGui::Text("Audio with that filename is not a file");
                                if (merge_output_name.empty()) ImGui::Text("Input the output filename");
                                ImGui::EndTooltip();
                            }

                            ImGui::EndDisabled();
                            
                            ImGui::EndDisabled();

                            ImGui::EndTabItem();
                        }

                        if (ImGui::BeginTabItem("Raw FFmpeg")) {
                            ImGui::BeginDisabled(!uv::recorder::process_done());
                            
                            ImGui::Text("Variables:");
                            ImGui::Bullet();
                            ImGui::Text("{showcases} - Path to a showcases folder");
                            
                            ImGui::Spacing();

                            ImGui::Text("ffmpeg.exe -y");
                            ImGui::InputTextMultiline("##Arguments Raw", &raw_arguments, { ImGui::GetContentRegionAvail().x, std::max(ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeight() * 2.0f - ImGui::GetStyle().FramePadding.y * 4.0f - ImGui::GetStyle().WindowPadding.y * 2.0f, ImGui::GetTextLineHeight()) });
                           
                            ImGui::Spacing();
                            
                            if (ImGui::Button("Open Showcases folder", { ImGui::GetContentRegionAvail().x, 0 })) geode::utils::file::openFolder(uv::showcase_path);

                            #define replace(s, h, n) do { \
                                pos = (s).find((h)); \
                                if (pos != std::string::npos) (s).replace(pos, sizeof(h)-1, (n)); \
                            } while (pos != std::string::npos)
                            
                            if (ImGui::Button("Run", { ImGui::GetContentRegionAvail().x, 0 })) {
                                std::string args = raw_arguments;
                                size_t pos;
                                replace(args, "{showcases}", geode::utils::string::pathToString(uv::showcase_path));
                                replace(args, "\n", " ");
                                uv::recorder::raw(args);
                            }

                            ImGui::EndDisabled();

                            ImGui::EndTabItem();
                        }

                        ImGui::EndTabBar();
                    }
                }
                
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Settings")) {
                ImGui::SeparatorText("Menu Keybind");

                ImGui::Dummy(ImVec2(0, ImGui::GetFrameHeight() - ImGui::GetTextLineHeightWithSpacing()));
                ImGui::Text("Toggle:");
                ImGui::SameLine();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetFrameHeight() + ImGui::GetTextLineHeightWithSpacing());
                keycode_selector("menu-keybind", GLFW_KEY_RIGHT_SHIFT);
                
                int keybind_mods = uv::hacks::get<int>("menu-keybind-mods", 0);

                ImGui::Dummy(ImVec2(0, ImGui::GetFrameHeight() - ImGui::GetTextLineHeightWithSpacing()));
                ImGui::Text("Modifiers:");
                ImGui::SameLine();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetFrameHeight() + ImGui::GetTextLineHeightWithSpacing());
                ImGui::CheckboxFlags("Shift", &keybind_mods, GLFW_MOD_SHIFT);
                ImGui::SameLine();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetFrameHeight() + ImGui::GetTextLineHeightWithSpacing());
                ImGui::CheckboxFlags("Control", &keybind_mods, GLFW_MOD_CONTROL);
                ImGui::SameLine();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetFrameHeight() + ImGui::GetTextLineHeightWithSpacing());
                ImGui::CheckboxFlags("Alt", &keybind_mods, GLFW_MOD_ALT);
                
                uv::hacks::set<int>("menu-keybind-mods", keybind_mods);
                
                ImGui::SeparatorText("Frame Stepper Keybinds");
                
                ImGui::Dummy(ImVec2(0, ImGui::GetFrameHeight() - ImGui::GetTextLineHeightWithSpacing()));
                ImGui::Text("Toggle:");
                ImGui::SameLine();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetFrameHeight() + ImGui::GetTextLineHeightWithSpacing());
                keycode_selector("frame-stepper-toggle-keybind", GLFW_KEY_C);

                ImGui::Dummy(ImVec2(0, ImGui::GetFrameHeight() - ImGui::GetTextLineHeightWithSpacing()));
                ImGui::Text("Step:");
                ImGui::SameLine();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetFrameHeight() + ImGui::GetTextLineHeightWithSpacing());
                keycode_selector("frame-stepper-step-keybind", GLFW_KEY_V);
                
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("About")) {
                ImGui::SeparatorText("About uvBot");

                ImGui::PushTextWrapPos(0.0f);

                ImGui::Text("uvBot is a frame GD bot made by aciddev_");
                ImGui::Text("uv is FOSS software - if you paid for it I suggest making a refund.\n"
                "The only official way to obtain uvBot is through GitHub Actions, Github Releases or Geode Index."
                " If you got your copy of uv somewhere else it is highly recommended you delete this version,"
                " as there is a chance that it was tampered with. (e.g. malware/stealers included)");
                ImGui::Text("The source code is under a Public Domain - this means you can do anything"
                " you want with it. Copy it, get inspired from it, use parts of it in your paid mod, etc.");
                ImGui::Text("\nThank you for using uvBot <3\nIf you want to - give it a star on GitHub\n");
                
                ImGui::TextLinkOpenURL("GitHub repository", "https://github.com/thisisignitedoreo/uvbot");
                ImGui::TextLinkOpenURL("aciddev_'s GitHub", "https://github.com/thisisignitedoreo");
                ImGui::TextLinkOpenURL("aciddev_'s YouTube", "https://youtube.com/@aciddev_");
                ImGui::TextLinkOpenURL("aciddev_'s Telegram", "https://t.me/aciddevv");
                ImGui::Text("aciddev_'s Discord: aciddev_");

                ImGui::PopTextWrapPos();
                
                ImGui::EndTabItem();
            }

            if (debug && ImGui::BeginTabItem("Debug")) {
                if (ImGui::Button(show_demo ? "Hide Demo window" : "Show Demo window")) show_demo = !show_demo;
                if (ImGui::Button(show_style_editor ? "Hide Style Editor window" : "Show Style Editor window")) show_style_editor = !show_style_editor;

                ImGui::Text("Last key pressed: %d", _last_key);
                ImGui::Text("Frame: %d", uv::bot::get_frame());
                
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    
    void draw(void) {
        bool animating = std::chrono::steady_clock::now() - toggle_time < animation_duration;
        if (!show && !animating) return;

        if (animating) {
            float alpha_value = (std::chrono::steady_clock::now() - toggle_time).count() / static_cast<float>(animation_duration.count());
            if (!show) alpha_value = 1.0f - alpha_value;
            // Apply Cubic InOut easing
            float intermediate = (-2 * alpha_value) + 2;
            alpha_value = alpha_value < 0.5f ? 4 * alpha_value * alpha_value * alpha_value : 1 - (intermediate * intermediate * intermediate) / 2;
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha_value);
        }

        if (uv::bot::current_state == uv::bot::state::recording) {
            ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.23f, 0.13f, 0.13f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.27f, 0.17f, 0.17f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.23f, 0.13f, 0.13f, 1.0f));
            pushed_colors = true;
        } else if (uv::bot::current_state == uv::bot::state::playing) {
            ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.13f, 0.23f, 0.13f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.17f, 0.27f, 0.17f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.13f, 0.23f, 0.13f, 1.0f));
            pushed_colors = true;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(400.0f, 200.0f));

        int flags = uv::bot::physic_player_1_actions.size() ? ImGuiWindowFlags_UnsavedDocument : 0;
        if (uv::bot::current_state == uv::bot::state::recording) ImGui::Begin("uvBot - Recording###uvBot", nullptr, flags);
        else if (uv::bot::current_state == uv::bot::state::playing) ImGui::Begin("uvBot - Playing###uvBot", nullptr, flags);
        else ImGui::Begin("uvBot###uvBot", nullptr, flags);

        if (uv::installed_conflict_mods.empty()) draw_main_ui();
        else {
            ImGui::PushTextWrapPos(0.0f);
            ImGui::Text("Some currently installed and enabled mods conflict with uvBot. Uninstall or disable them to use this mod.");
            ImGui::Text("Conflicting mods:");
            for (auto &i : uv::installed_conflict_mods) {
                ImGui::Bullet();
                ImGui::Text(i.c_str());
            }
            ImGui::PopTextWrapPos();
        }
        
        ImGui::End();

        ImGui::PopStyleVar();
        
        if (pushed_colors) {
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            pushed_colors = false;
        }

        if (show_demo) ImGui::ShowDemoWindow();
        if (show_style_editor) ImGui::ShowStyleEditor();

        if (animating) ImGui::PopStyleVar();
    }
}

class $modify(cocos2d::CCEGLView) {
    void onGLFWKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        CCEGLView::onGLFWKeyCallback(window, key, scancode, action, mods);

        if (action == GLFW_PRESS) {
            if (key == uv::hacks::get<int>("menu-keybind", GLFW_KEY_RIGHT_SHIFT) && mods == uv::hacks::get<int>("menu-keybind-mods", 0)) {
                uv::gui::toggle_time = std::chrono::steady_clock::now();
                uv::gui::show = !uv::gui::show;
            }
            if (key == uv::hacks::get<int>("frame-stepper-toggle-keybind", GLFW_KEY_C) && mods == 0) uv::hacks::frame_stepper::on = !uv::hacks::frame_stepper::on;
            if (key == uv::hacks::get<int>("frame-stepper-step-keybind", GLFW_KEY_V) && mods == 0) uv::hacks::frame_stepper::step_for++;
            
            if (key == GLFW_KEY_7 && mods == GLFW_MOD_CONTROL && uv::gui::show) uv::gui::debug = !uv::gui::debug;
            
            uv::gui::_last_key = key;
            uv::gui::key = key;
        }
    }
};
