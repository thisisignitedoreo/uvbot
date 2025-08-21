#include <subprocess.hpp>

#include "common.hh"

// Thanks matcool
namespace uv::bot::recorder {
    struct render_texture {
        unsigned int width, height;
        unsigned int fbo; int old_fbo;
        unsigned int texture_id;
        cocos2d::CCTexture2D *texture;
    };

    static render_texture rt;

    bool recording = false;
    static bool frame_has_data = false;
    std::mutex mutex;

    options recording_options;

    void *frame_buffer = nullptr;
    
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
        rt.texture->autorelease();

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
                "ffmpeg.exe", "-y",
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
            
            std::string command = fmt::format("\"{}\" ", fmt::join(cmd, "\" \""));
            command += opt.custom_options;
            command += " \"" + opt.output_path + "\"";
            
            auto process = subprocess::Popen(command);
            while ((recording || frame_has_data) && !process.has_exited()) {
                mutex.lock();
                if (frame_has_data) {
                    process.m_stdin.write(frame_buffer, opt.width * opt.height * 3);
                    frame_has_data = false;
                }
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
}
