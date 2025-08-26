#include <chrono>
#include <cmath>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "common.hh"

namespace uv::gui {
    bool show = false;
    bool debug = false;
    std::chrono::steady_clock::time_point toggle_time;
    
    static bool show_demo = false, show_style_editor = false, pushed_colors = false, hitboxes_colors_opened = false;
    
    static std::string macro_name, video_name, audio_name;

    static const std::filesystem::path macro_path = geode::Mod::get()->getSaveDir() / "Macros";
    static const std::filesystem::path showcase_path = geode::Mod::get()->getSaveDir() / "Showcases";
    
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
        .output_path = geode::utils::string::pathToString(showcase_path / ".mp4"),
        .custom_options = "-pix_fmt yuv420p -vf \"vflip\"",
        .hide_end_level_screen = true,
        .fade_out = false,
    };
    
    uv::recorder::audio::options audio_opts = {
        .output_path = geode::utils::string::pathToString(showcase_path / ".wav"),
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

    static void draw_persistent(void) {
        // TODO: write this part
    }
    
    void draw(void) {
        draw_persistent();
        
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

        if (uv::bot::current_state == uv::bot::state::recording) ImGui::Begin("uvBot - Recording###uvBot");
        else if (uv::bot::current_state == uv::bot::state::playing) ImGui::Begin("uvBot - Playing###uvBot");
        else ImGui::Begin("uvBot###uvBot");

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
                
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::InputTextWithHint("##Macro Name", "Macro Name", &macro_name);

                float button_widths = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2) / 3;
                
                ImGui::BeginDisabled(macro_name.empty());
                if (ImGui::Button("Save", { button_widths, 0 })) uv::bot::save(geode::utils::string::trim(macro_name));
                if (macro_name.empty() && ImGui::IsItemHovered() && ImGui::BeginTooltip()) {
                    ImGui::Text("To save a macro, input its name");
                    ImGui::EndTooltip();
                }
                ImGui::SameLine();
                if (ImGui::Button("Load", { button_widths, 0 })) uv::bot::load(geode::utils::string::trim(macro_name)); 
                if (macro_name.empty() && ImGui::IsItemHovered() && ImGui::BeginTooltip()) {
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
                
                if (ImGui::Button("Open Macros folder", { ImGui::GetContentRegionAvail().x, 0 })) geode::utils::file::openFolder(macro_path);

                ImGui::SeparatorText("Macros");

                bool not_empty = false;
                std::error_code error;
                for (auto const &dir_entry : std::filesystem::directory_iterator(macro_path, error)) {
                    not_empty = true; // I am too lazy to do this the right way
                    if (dir_entry.is_regular_file() && geode::utils::string::pathToString(dir_entry.path()).ends_with(".uv")) {
                        std::string path = geode::utils::string::pathToString(dir_entry.path());
                        int from = path.rfind(dir_entry.path().preferred_separator);
                        if (from == std::string::npos) from = 0;
                        std::string name = path.substr(from + 1, path.size() - from - 4);
                        if (ImGui::Button(name.c_str(), { ImGui::GetContentRegionAvail().x, 0 })) {
                            uv::bot::load(name);
                            macro_name = name;
                        }
                    }
                }

                if (!not_empty) ImGui::TextDisabled("No macros saved... :_(");

                if (error) geode::log::debug("Error reading macro directory: {}", error.message());

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Hacks")) {
                ImGui::SeparatorText("Botting");
                
                ImGui::Checkbox("##Speedhack Checkbox", &uv::hacks::speedhack);
                ImGui::SameLine();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::DragFloat("##Speedhack", &uv::hacks::speedhack_multiplier, 0.01f, 0.0f, 3.0f, "Speedhack: %.2fx");
                if (uv::hacks::speedhack_multiplier <= 0.0f) uv::hacks::speedhack_multiplier = 1.0f;
                
                ImGui::Checkbox("Classic Mode Speedhack", &uv::hacks::speedhack_classic);
                ImGui::SameLine();
                ImGui::TextDisabled("?");
                ImGui::SetItemTooltip("Instead of making the game smoothly slower, classic mode slows down the framerate.\nUseful for recording the game with OBS or something.");

                ImGui::Checkbox("Lock DeltaTime", &uv::hacks::lock_delta);
                ImGui::SameLine();
                ImGui::TextDisabled("?");
                ImGui::SetItemTooltip("Disables frame skip when the game can't keep up with the target FPS.\nUseful for showcasing.");
                
                ImGui::Checkbox("Practice Fix", &uv::hacks::practice_fix);

                ImGui::SeparatorText("Player");

                if (ImGui::Checkbox("Noclip", &uv::hacks::noclip) && uv::hacks::noclip) {
                    uv::hacks::noclip_p1 = false;
                    uv::hacks::noclip_p2 = false;
                }
                ImGui::SameLine();
                if (ImGui::Checkbox("Player 1", &uv::hacks::noclip_p1) && uv::hacks::noclip_p1) {
                    uv::hacks::noclip = false;
                    uv::hacks::noclip_p2 = false;
                }
                ImGui::SameLine();
                if (ImGui::Checkbox("Player 2", &uv::hacks::noclip_p2) && uv::hacks::noclip_p2) {
                    uv::hacks::noclip_p1 = false;
                    uv::hacks::noclip = false;
                }
                
                ImGui::SeparatorText("Hitboxes");
                
                ImGui::Checkbox("Show Hitboxes", &uv::hacks::hitboxes);
                
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::DragFloat("##Hitboxes Thickness", &uv::hacks::hitboxes_thickness, 0.01f, 0.01f, 3.0f, "Thickness: %.2f", ImGuiSliderFlags_ClampOnInput);

                if (ImGui::ArrowButton("Hitboxes Colors Collapser", hitboxes_colors_opened ? ImGuiDir_Down : ImGuiDir_Right)) hitboxes_colors_opened = !hitboxes_colors_opened;
                ImGui::SameLine();
                ImGui::Text("Hitboxes Colors");
                
                if (hitboxes_colors_opened) {
                    ImGui::ColorEdit4("Hazards", uv::hacks::hitboxes_color_hazards.data());
                    ImGui::ColorEdit4("Solids", uv::hacks::hitboxes_color_solids.data());
                    ImGui::ColorEdit4("Specials", uv::hacks::hitboxes_color_specials.data());
                    ImGui::ColorEdit4("Player", uv::hacks::hitboxes_color_player.data());
                    ImGui::Spacing();
                    ImGui::ColorEdit4("Hazards Fill", uv::hacks::hitboxes_color_fill_hazards.data());
                    ImGui::ColorEdit4("Solids Fill", uv::hacks::hitboxes_color_fill_solids.data());
                    ImGui::ColorEdit4("Specials Fill", uv::hacks::hitboxes_color_fill_specials.data());
                    ImGui::ColorEdit4("Player Fill", uv::hacks::hitboxes_color_fill_player.data());
                }
                    
                ImGui::SeparatorText("Cosmetic");
                
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::Checkbox("Layout Mode", &uv::hacks::layout_mode);
                
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
                                render_opts.output_path = geode::utils::string::pathToString(showcase_path / video_name);
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
                            
                            if (ImGui::Button("Open Showcases folder", { ImGui::GetContentRegionAvail().x, 0 })) geode::utils::file::openFolder(showcase_path);

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
                            
                            ImGui::EndDisabled();

                            if (disabled && ImGui::IsItemHovered() && ImGui::BeginTooltip()) {
                                if (video_name.empty()) ImGui::Text("Input the video filename");
                                ImGui::EndTooltip();
                            }

                            ImGui::EndTabItem();
                        }

                        if (ImGui::BeginTabItem("Audio")) {
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            if (ImGui::InputTextWithHint("##Audio Filename", "Audio Filename (e.g. file.wav)", &audio_name)) {
                                audio_opts.output_path = geode::utils::string::pathToString(showcase_path / audio_name);
                            }

                            ImGui::Spacing();

                            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
                            ImGui::DragFloat("##Music volume", &audio_opts.music_volume, 0.01f, 0.0f, 1.0f, "Music volume: %.2f");
                            ImGui::DragFloat("##SFX volume", &audio_opts.sfx_volume, 0.01f, 0.0f, 1.0f, "SFX volume: %.2f");
                            ImGui::DragFloat("##Excess render", &audio_opts.excess_render, 0.1f, 0.0f, 5.0f, "Render after level ends: %.1fs");
                            ImGui::PopItemWidth();

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
                            
                            ImGui::EndDisabled();

                            if (disabled && ImGui::IsItemHovered() && ImGui::BeginTooltip()) {
                                if (audio_name.empty()) ImGui::Text("Input the audio filename");
                                else if (!audio_name.ends_with(".wav")) ImGui::Text("Only .wav format is supported right now");
                                ImGui::EndTooltip();
                            }

                            ImGui::EndTabItem();
                        }

                        ImGui::EndTabBar();
                    }
                }
                
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

                if (ImGui::Button("WAVWRITER")) FMODAudioEngine::sharedEngine()->m_system->setOutput(FMOD_OUTPUTTYPE_WAVWRITER);
                if (ImGui::Button("AUTODETECT")) FMODAudioEngine::sharedEngine()->m_system->setOutput(FMOD_OUTPUTTYPE_AUTODETECT);
                
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
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
