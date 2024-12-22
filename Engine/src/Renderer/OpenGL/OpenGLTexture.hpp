#ifndef SRC_RENDERER_OPENGL_TEXTURE_HPP
#define SRC_RENDERER_OPENGL_TEXTURE_HPP

#include <glad/glad.h>

#include <filesystem>

#include "../Texture.hpp"
#include "../../../dependencies/lodepng/lodepng.h"

namespace nb
{
    namespace OpenGl
    {
        class OpenGlTexture : Renderer::Texture
        {
            public:
                OpenGlTexture(const std::filesystem::path& path) noexcept;
                ~OpenGlTexture() noexcept;

                void bind(const uint32_t slotId) const noexcept;

            private:
                GLuint texture;
        };
    };
};

#endif