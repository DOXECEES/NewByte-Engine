#ifndef SRC_RENDERER_RENDERERTRACKER_HPP
#define SRC_RENDERER_RENDERERTRACKER_HPP

#include "IFrameBuffer.hpp"

#include <unordered_map>
#include <string_view>
#include <memory>

namespace nb::Renderer
{
    class RendererTracker
    {
    public:
        

        static void addFrameBuffer(std::string_view name, std::shared_ptr<IFrameBuffer> frameBuffer) noexcept
        {
            framebuffers[name] = frameBuffer;
        }

        static std::shared_ptr<IFrameBuffer> getFrameBuffer(std::string_view name) noexcept
        {
            return framebuffers[name];
        }


    private:
        inline static std::unordered_map<std::string_view, std::shared_ptr<IFrameBuffer>> framebuffers;

    };

};

#endif