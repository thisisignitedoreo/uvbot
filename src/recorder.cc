#include <subprocess.hpp>

#include <Geode/Geode.hpp>

#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/UILayer.hpp>

#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

#include "common.hh"

// Thanks matcool
namespace uv::recorder {
    struct render_texture {
        unsigned int width, height;
        unsigned int fbo; int old_fbo;
        unsigned int texture_id;
        cocos2d::CCTexture2D *texture;
    };

    bool recording = false;
    std::mutex mutex;

    options recording_options;

    static render_texture rt;

    static void *frame_buffer = nullptr;
    static bool frame_has_data = false;

    static subprocess::Popen process;
    static bool process_running = false;
    
    void begin_rt(unsigned int width, unsigned int height) {
        rt.width = width;
        rt.height = height;
        
        glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &rt.old_fbo);

        rt.texture = new cocos2d::CCTexture2D();
        
        std::size_t texture_size = rt.width * rt.height * 3;
        void *data = std::malloc(texture_size);
        std::memset(data, 0, texture_size);

        rt.texture->initWithData(data, cocos2d::CCTexture2DPixelFormat::kCCTexture2DPixelFormat_RGB888, rt.width, rt.height, cocos2d::CCSize(static_cast<float>(rt.width), static_cast<float>(rt.height)));

        std::free(data);

        int old_rbo = 0;
        glGetIntegerv(GL_RENDERBUFFER_BINDING_EXT, &old_rbo);
        
        glGenFramebuffersEXT(1, &rt.fbo);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rt.fbo);

        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, rt.texture->getName(), 0);
        
        rt.texture->setAliasTexParameters();
        
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, old_rbo);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rt.old_fbo);
    }

    void capture_rt(std::mutex &mutex, void *data) {
        glViewport(0, 0, rt.width, rt.height);

        glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &rt.old_fbo);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rt.fbo);

        PlayLayer *pl = PlayLayer::get();
        if (pl) pl->visit();

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        mutex.lock();
        glReadPixels(0, 0, rt.width, rt.height, GL_RGB, GL_UNSIGNED_BYTE, data);
        frame_has_data = true;
        mutex.unlock();

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rt.old_fbo);
        cocos2d::CCDirector *director = cocos2d::CCDirector::get();
        director->setViewport();
    }

    void end_rt(void) {
        rt.texture->release();
    }

    void start(options opt) {
        begin_rt(opt.width, opt.height);

        frame_buffer = std::malloc(opt.width * opt.height * 3);
        recording = true;
        recording_options = opt;

        std::thread([&, opt]() {
            std::vector<std::string> cmd = {
                (geode::dirs::getGameDir() / "ffmpeg.exe").string(), "-y",
                "-f", "rawvideo", "-pix_fmt", "rgb24",
                "-s", fmt::format("{}x{}", opt.width, opt.height),
                "-r", std::to_string(opt.fps),
                "-i", "-",
            };
            
            if (opt.codec.size()) {
                cmd.push_back("-c:v");
                cmd.push_back(opt.codec);
            }
            if (opt.bitrate.size()) {
                cmd.push_back("-b:v");
                cmd.push_back(opt.bitrate);
            }
            
            std::string command = fmt::format("\"{}\" {} \"{}\"", fmt::join(cmd, "\" \""), opt.custom_options, opt.output_path);
            
            auto process = subprocess::Popen(command);
            while ((recording || frame_has_data) && !process.has_exited()) {
                mutex.lock();
                if (frame_has_data) {
                    process.m_stdin.write(frame_buffer, opt.width * opt.height * 3);
                }
                frame_has_data = false;
                mutex.unlock();
            }
            process.close();
        }).detach();
    }

    void update(void) {
        while (frame_has_data);
        
        capture_rt(mutex, frame_buffer);
    }
    
    void end(void) {
        std::free(frame_buffer);
        frame_buffer = nullptr;
        recording = false;
        end_rt();
    }

    void merge(std::string video, std::string audio, std::string output, std::string args) {
        std::vector<std::string> cmd = {
            (geode::dirs::getGameDir() / "ffmpeg.exe").string(), "-y",
            "-i", video, "-i", audio,
        };
        
        std::string command = fmt::format("\"{}\" {} \"{}\"", fmt::join(cmd, "\" \""), args, output);
        
        process = subprocess::Popen(command);
        process_running = true;
    }

    void raw(std::string args) {
        std::vector<std::string> cmd = {
            (geode::dirs::getGameDir() / "ffmpeg.exe").string(), "-y",
        };
        
        std::string command = fmt::format("\"{}\" {}", fmt::join(cmd, "\" \""), args);
        
        process = subprocess::Popen(command);
        process_running = true;
    }

    bool process_done(void) {
        if (process_running == false) return true;
        bool exited = process.has_exited();
        if (exited) {
            process.close();
            process_running = false;
        }
        return false;
    }
    
    namespace audio {
        bool recording = false;
        options recording_options;

        float original_music_volume, original_sfx_volume;

        static std::vector<float> data;
        static FMOD::DSP *dsp = nullptr;
        static FMOD::ChannelGroup *master_group = nullptr;
        static std::mutex data_mutex;

        // Thanks toby
        void init(options opt) {
            recording_options = opt;
            recording = true;
            
            auto system = FMODAudioEngine::get()->m_system;

            FMOD_DSP_DESCRIPTION desc = {};
            strcpy(desc.name, "DSP Recorder");
            desc.numinputbuffers = 1;
            desc.numoutputbuffers = 1;
            desc.version = FMOD_VERSION;
            desc.read = [](FMOD_DSP_STATE* dsp_state, float* in_buffer, float* out_buffer, unsigned int length, int in_channels, int* out_channels) {
                int channels = in_channels;

                std::memcpy(out_buffer, in_buffer, length * channels * sizeof(float));

                if (out_channels) *out_channels = channels;
                
                if (recording) {
                    data_mutex.lock();
                    data.insert(data.end(), in_buffer, in_buffer + length * channels);
                    data_mutex.unlock();
                }
                
                return FMOD_OK;
            };

            FMOD_RESULT r;
            r = system->createDSP(&desc, &dsp);
            assert(r == FMOD_OK);
            r = system->getMasterChannelGroup(&master_group);
            assert(r == FMOD_OK);
            geode::log::debug("init()");
        }

        void start(void) {
            FMODAudioEngine *fmod_engine = FMODAudioEngine::get();
            original_music_volume = fmod_engine->getBackgroundMusicVolume();
            original_sfx_volume = fmod_engine->getEffectsVolume();

            fmod_engine->setBackgroundMusicVolume(recording_options.music_volume);
            fmod_engine->setEffectsVolume(recording_options.sfx_volume);

            FMOD_RESULT r = master_group->addDSP(FMOD_CHANNELCONTROL_DSP_TAIL, dsp);
            assert(r == FMOD_OK);
            
            dsp->setActive(true);
            dsp->setBypass(false);

            data_mutex.lock();
            data.clear();
            data_mutex.unlock();
            geode::log::debug("start()");
        }

        static void save_to_wav(const char *output_path) {
            int sample_rate, channels;
            FMODAudioEngine::get()->m_system->getSoftwareFormat(&sample_rate, nullptr, &channels);
            
            drwav_data_format format;
            format.container = drwav_container_riff;
            format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
            format.channels = channels;
            format.sampleRate = sample_rate;
            format.bitsPerSample = 32;

            drwav wav;
            drwav_init_file_write(&wav, output_path, &format, NULL);

            drwav_uint64 written = drwav_write_pcm_frames(&wav, data.size() / channels, data.data());

            geode::log::debug("Written {} samples out of {}", written, data.size() / channels);

            drwav_uninit(&wav);
        }
        
        void end(void) {
            recording = false;
            
            master_group->removeDSP(dsp);

            FMODAudioEngine *fmod_engine = FMODAudioEngine::get();
            fmod_engine->setBackgroundMusicVolume(original_music_volume);
            fmod_engine->setEffectsVolume(original_sfx_volume);

            save_to_wav(recording_options.output_path.c_str());
            geode::log::debug("end()");
        }
    }
}

static bool level_ended = false;
static bool started_audio_render = false;
static std::chrono::steady_clock::time_point level_end_point;
static EndLevelLayer *ell;

class $modify(HookedEndLevelLayer, EndLevelLayer) {
    struct Fields {
        cocos2d::CCSprite *black_fg;
    };

    void update_fade_out(float) {
        if (m_fields->black_fg) {
            if (uv::recorder::recording) {
                std::chrono::steady_clock::duration excess(std::chrono::milliseconds(static_cast<long long>(uv::recorder::recording_options.excess_render * 1000)));
                float opacity = (std::chrono::steady_clock::now() - level_end_point).count() / static_cast<float>(excess.count());
                m_fields->black_fg->setOpacity(static_cast<unsigned char>(opacity * 255.0f));
            } else m_fields->black_fg->setOpacity(0);
        }
        if (!uv::recorder::recording) this->setVisible(true);
    }
    
    void showLayer(bool p0) {
        EndLevelLayer::showLayer(p0);

        if (uv::recorder::recording || uv::recorder::audio::recording) {
            level_ended = true;
            level_end_point = std::chrono::steady_clock::now();
            ell = this;

            PlayLayer *pl = PlayLayer::get();
            if (uv::recorder::recording && pl) {
                cocos2d::CCSize wnd_size = cocos2d::CCDirector::sharedDirector()->getWinSize();
                
                m_fields->black_fg = cocos2d::CCSprite::create("game_bg_13_001.png");
                
                cocos2d::CCSize spr_size = m_fields->black_fg->getContentSize();
                m_fields->black_fg->setPosition({ wnd_size.width / 2, wnd_size.height / 2 });            
                m_fields->black_fg->setScaleX(wnd_size.width / spr_size.width * 2.f);
                m_fields->black_fg->setScaleY(wnd_size.height / spr_size.height * 2.f);
                m_fields->black_fg->setColor({ 0, 0, 0 });
                m_fields->black_fg->setOpacity(0);
                m_fields->black_fg->setZOrder(1000);

                pl->addChild(m_fields->black_fg);
            }
        }
        
        this->schedule(schedule_selector(HookedEndLevelLayer::update_fade_out), 0.0f);
        this->setVisible(!uv::recorder::recording || !uv::recorder::recording_options.hide_end_level_screen);
    }
};

class $modify(GJBaseGameLayer) {
    struct Fields {
        float initial_x = -0.0f;
        bool ready_to_render = false;
    };

    void update(float dt) {
        // Weird hack to bypass start level lil delay
        if (m_fields->initial_x == -0.0f) m_fields->initial_x = this->m_player1->m_position.x;
        m_fields->ready_to_render = m_fields->initial_x < this->m_player1->m_position.x;
        
        if (!uv::recorder::recording && uv::recorder::audio::recording && !started_audio_render && m_fields->ready_to_render) {
            started_audio_render = true;
            uv::recorder::audio::start();
        }
        
        if (uv::recorder::recording && m_fields->ready_to_render) {
            PlayLayer *pl = PlayLayer::get();
            if (pl) pl->processActivatedAudioTriggers(pl->m_gameState.m_levelTime);
            uv::recorder::update();
            if (level_ended) {
                std::chrono::steady_clock::duration excess_amount(std::chrono::milliseconds(static_cast<long long>(uv::recorder::recording_options.excess_render * 1000)));
                if (std::chrono::steady_clock::now() - level_end_point >= excess_amount) {
                    level_ended = false;
                    uv::recorder::end();
                }
            }
        }
        
        if (uv::recorder::audio::recording) {
            if (level_ended) {
                std::chrono::steady_clock::duration excess_amount(std::chrono::milliseconds(static_cast<long long>(uv::recorder::audio::recording_options.excess_render * 1000)));
                if (std::chrono::steady_clock::now() - level_end_point >= excess_amount) {
                    level_ended = false;
                    started_audio_render = false;
                    uv::recorder::audio::end();
                }
            }
        }
        
        GJBaseGameLayer::update(dt);
    }
};

class $modify(cocos2d::CCScheduler) {
    void update(float dt) {
        if (uv::recorder::recording) {
            cocos2d::CCScheduler::update(1.0f / uv::recorder::recording_options.fps);
        } else {
            cocos2d::CCScheduler::update(uv::hacks::get("lock-delta", false) ? cocos2d::CCDirector::get()->getAnimationInterval() : dt);
        }
    }
};

// Thanks toby
class $modify(UILayer) {
    bool init(GJBaseGameLayer *p0) {
        if (!UILayer::init(p0)) return false;
        
        if (!uv::recorder::recording) return true;
        
        cocos2d::CCMenu *menu = getChildByType<cocos2d::CCMenu>(0);
        CCMenuItemSpriteExtra *btn = menu->getChildByType<CCMenuItemSpriteExtra>(0);

        if (menu && btn) btn->getNormalImage()->setVisible(false);

        return true;
    }
};
