#ifndef SRC_RENDERER_OPENGL_TEXTURE_HPP
#define SRC_RENDERER_OPENGL_TEXTURE_HPP

#include <glad/glad.h>

#include <filesystem>

#include "../Texture.hpp"
#include "../../../dependencies/lodepng/lodepng.h"
#include "NbCore.hpp"

namespace nb
{
    namespace OpenGl
    {
        class OpenGlTexture : public Renderer::Texture
        {
            public:
                OpenGlTexture(const std::filesystem::path& path) noexcept;
                ~OpenGlTexture() noexcept;

                NB_NON_COPYABLE(OpenGlTexture);
                NB_MOVABLE(OpenGlTexture);

                void bind(const uint32_t slotId) const noexcept;

                uint32_t getId() const noexcept override;

            private:
                GLuint texture;
        };
    };
};

#endif