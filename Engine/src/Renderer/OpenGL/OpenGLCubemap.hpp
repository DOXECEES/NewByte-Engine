#ifndef SRC_RENDERER_OPENGL_OPENGLCUBEMAP_HPP
#define SRC_RENDERER_OPENGL_OPENGLCUBEMAP_HPP

#include <cstdint>
#include "Renderer/Cubemap.hpp"

namespace nb::OpenGl
{
    class OpenGLCubemap : public Renderer::Cubemap
    {
    public:
        OpenGLCubemap() noexcept;
        OpenGLCubemap(uint32_t size, int internalFormat) noexcept;
        ~OpenGLCubemap();

        void bind(uint32_t slot = 0) const noexcept;
        void unbind() const noexcept;

        void finalizeBindless() noexcept;


        uint32_t getId() const noexcept override final;
        uint64_t getHandle() const noexcept override final;

    private: 
        uint64_t handle     = 0;
        uint32_t envCubemap = 0;
        uint32_t size       = 0;
        
    };
}

#endif