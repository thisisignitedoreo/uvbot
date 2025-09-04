#include "common.hh"

namespace uv::bot::foreign {
    struct RE_physic {
        unsigned int frame;
        float x;
        float y;
        float rot;
        double y_velocity;
        bool player_1;
    };

    struct RE_input {
        unsigned int frame;
        bool hold;
        int button;
        bool player_1;
    };

    std::vector<std::string> supported_exts = {
        ".re3",
        ".slc",
    };

    #define READ(f, t, p) (f).read(reinterpret_cast<char*>(p), sizeof(t))

    static error load_re3(std::ifstream &f) {
        float tps;
        READ(f, float, &tps);

        uv::hacks::set("tps", std::max(240.0f, tps));

        unsigned int p1_size, p2_size, p1_input_size, p2_input_size;
        READ(f, unsigned int, &p1_size);
        READ(f, unsigned int, &p2_size);
        READ(f, unsigned int, &p1_input_size);
        READ(f, unsigned int, &p2_input_size);

        for (unsigned int i = 0; i < p1_size + p2_size; i++) {
            RE_physic src_act;
            READ(f, RE_physic, &src_act);

            uv::bot::physic_action act;
            act.frame = src_act.frame;
            act.x = src_act.x;
            act.y = src_act.y;
            act.rotation = src_act.rot;
            act.y_velocity = src_act.y_velocity;

            if (i < p1_size) uv::bot::physic_player_1_actions.push_back(act);
            else uv::bot::physic_player_2_actions.push_back(act);
        }

        // Intermix P1 + P2
        std::map<std::uint64_t, uv::bot::input_action> inputs;
        
        for (unsigned int i = 0; i < p1_input_size + p2_input_size; i++) {
            RE_input src_act;
            READ(f, RE_input, &src_act);

            uv::bot::input_action act;
            act.frame = src_act.frame;
            act.flags = ((src_act.button - 1 + (3 - src_act.player_1 * 3)) << 1) + src_act.hold;

            inputs[src_act.frame] = act;
        }

        for (auto &i : inputs) uv::bot::input_actions.push_back(i.second);

        return error::none;
    }

    static error load_slc_v1(std::ifstream &f) {
        double tps;
        READ(f, double, &tps);
        uv::hacks::set<float>("tps", static_cast<float>(tps));

        std::uint32_t action_count;
        READ(f, std::uint32_t, &action_count);

        for (std::uint32_t i = 0; i < action_count; i++) {
            std::uint32_t action;
            READ(f, std::uint32_t, &action);

            uv::bot::input_action act;
            act.frame = action >> 4;
            int p2 = (action & 0b1000) > 0;
            int button = ((action & 0b110) >> 1) - 1;
            int hold = action & 0b0001;
            act.flags = (((p2 * 3) + button) << 1) + hold;
            
            uv::bot::input_actions.push_back(act);
        }

        if (!f.eof()) {
            // TODO: add random seed
        }

        return error::none;
    }
    
    static error load_slc(std::ifstream &f) {
        char magic[4];
        f.read(magic, 4);
        f.seekg(0);
        if (memcmp(magic, "SILL", 4)) return error::unsupported_slc_v2;
        else if (memcmp(magic, "SLC3", 4)) return error::unsupported_slc_v3;
        return load_slc_v1(f);
    }
    
    error load(std::string name) {
        uv::bot::clear();
        std::ifstream f(uv::macro_path / name, std::ios::binary);

        if (name.ends_with(".re3")) return load_re3(f);
        if (name.ends_with(".slc")) return load_slc(f);
        return error::unsupported;
    }
}
