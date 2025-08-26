
#include "common.hh"

namespace uv::hacks {
    bool speedhack;
    bool speedhack_classic;
    float speedhack_multiplier;
    bool lock_delta;
    bool noclip;
    bool noclip_p1;
    bool noclip_p2;
    bool hitboxes;
    float hitboxes_thickness;
    std::vector<float> hitboxes_color_hazards, hitboxes_color_fill_hazards;
    std::vector<float> hitboxes_color_solids, hitboxes_color_fill_solids;
    std::vector<float> hitboxes_color_specials, hitboxes_color_fill_specials;
    std::vector<float> hitboxes_color_player, hitboxes_color_fill_player;
    bool layout_mode;
    bool practice_fix;
    
    void save(void) {
        geode::Mod::get()->setSavedValue<bool>("speedhack", speedhack);
        geode::Mod::get()->setSavedValue<bool>("speedhack-classic", speedhack_classic);
        geode::Mod::get()->setSavedValue<float>("speedhack-multiplier", speedhack_multiplier);
        geode::Mod::get()->setSavedValue<bool>("lock-delta", lock_delta);
        geode::Mod::get()->setSavedValue<bool>("noclip", noclip);
        geode::Mod::get()->setSavedValue<bool>("noclip-p1", noclip_p1);
        geode::Mod::get()->setSavedValue<bool>("noclip-p2", noclip_p2);
        geode::Mod::get()->setSavedValue<bool>("hitboxes", hitboxes);
        geode::Mod::get()->setSavedValue<float>("hitboxes-thickness", hitboxes_thickness);
        geode::Mod::get()->setSavedValue<std::vector<float>>("hitboxes-color-hazards", hitboxes_color_hazards);
        geode::Mod::get()->setSavedValue<std::vector<float>>("hitboxes-color-fill-hazards", hitboxes_color_fill_hazards);
        geode::Mod::get()->setSavedValue<std::vector<float>>("hitboxes-color-solids", hitboxes_color_solids);
        geode::Mod::get()->setSavedValue<std::vector<float>>("hitboxes-color-fill-solids", hitboxes_color_fill_solids);
        geode::Mod::get()->setSavedValue<std::vector<float>>("hitboxes-color-specials", hitboxes_color_specials);
        geode::Mod::get()->setSavedValue<std::vector<float>>("hitboxes-color-fill-specials", hitboxes_color_fill_specials);
        geode::Mod::get()->setSavedValue<std::vector<float>>("hitboxes-color-player", hitboxes_color_player);
        geode::Mod::get()->setSavedValue<std::vector<float>>("hitboxes-color-fill-player", hitboxes_color_fill_player);
        geode::Mod::get()->setSavedValue<bool>("layout-mode", layout_mode);
        geode::Mod::get()->setSavedValue<bool>("practice-fix", practice_fix);
    }

    void load(void) {
        speedhack = geode::Mod::get()->getSavedValue<bool>("speedhack", false);
        speedhack_classic = geode::Mod::get()->getSavedValue<bool>("speedhack-classic", false);
        speedhack_multiplier = geode::Mod::get()->getSavedValue<float>("speedhack-multiplier", 1.0f);
        lock_delta = geode::Mod::get()->getSavedValue<bool>("lock-delta", false);
        noclip = geode::Mod::get()->getSavedValue<bool>("noclip", false);
        noclip_p1 = geode::Mod::get()->getSavedValue<bool>("noclip-p1", false);
        noclip_p2 = geode::Mod::get()->getSavedValue<bool>("noclip-p2", false);
        hitboxes = geode::Mod::get()->getSavedValue<bool>("hitboxes", false);
        hitboxes_thickness = geode::Mod::get()->getSavedValue<float>("hitboxes-thickness", 0.3f);
        hitboxes_color_hazards = geode::Mod::get()->getSavedValue<std::vector<float>>("hitboxes-color-hazards", { 1.0f, 0.0f, 0.0f, 1.0f });
        hitboxes_color_fill_hazards = geode::Mod::get()->getSavedValue<std::vector<float>>("hitboxes-color-fill-hazards", { 1.0f, 0.0f, 0.0f, 0.2f });
        hitboxes_color_solids = geode::Mod::get()->getSavedValue<std::vector<float>>("hitboxes-color-solids", { 0.0f, 0.0f, 1.0f, 1.0f });
        hitboxes_color_fill_solids = geode::Mod::get()->getSavedValue<std::vector<float>>("hitboxes-color-fill-solids", { 0.0f, 0.0f, 1.0f, 0.2f });
        hitboxes_color_specials = geode::Mod::get()->getSavedValue<std::vector<float>>("hitboxes-color-specials", { 0.0f, 1.0f, 0.0f, 1.0f });
        hitboxes_color_fill_specials = geode::Mod::get()->getSavedValue<std::vector<float>>("hitboxes-color-fill-specials", { 0.0f, 1.0f, 0.0f, 0.2f });
        hitboxes_color_player = geode::Mod::get()->getSavedValue<std::vector<float>>("hitboxes-color-player", { 1.0f, 1.0f, 0.0f, 1.0f });
        hitboxes_color_fill_player = geode::Mod::get()->getSavedValue<std::vector<float>>("hitboxes-color-fill-player", { 1.0f, 1.0f, 0.0f, 0.2f });
        layout_mode = geode::Mod::get()->getSavedValue<bool>("layout-mode", false);
        practice_fix = geode::Mod::get()->getSavedValue<bool>("practice-fix", true);
    }
}
