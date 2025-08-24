#include "common.hh"

#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

namespace uv::recorder::audio {
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

            geode::log::debug("Sanity check - callback is called");

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
