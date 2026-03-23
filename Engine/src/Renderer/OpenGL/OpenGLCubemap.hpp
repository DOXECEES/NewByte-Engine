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

        uint32_t getId() const noexcept override { return envCubemap; }

    private:
        uint32_t envCubemap = 0;
    };
}

#endif