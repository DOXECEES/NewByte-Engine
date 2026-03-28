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
                OpenGlTexture(
                    int width,
                    int height,
                    GLint internalFormat,
                    GLenum dataFormat,
                    GLenum dataType,
                    void* data
                ) noexcept;

                OpenGlTexture(GLuint id, uint32_t w, uint32_t h) noexcept
                    : texture(id),
                      width(w),
                      height(h)
                {
                    
                }


                ~OpenGlTexture() noexcept;

                NB_NON_COPYABLE(OpenGlTexture);
                NB_MOVABLE(OpenGlTexture);

                void bind(uint32_t slotId) override
                {
                    glActiveTexture(GL_TEXTURE0 + slotId);
                    glBindTexture(GL_TEXTURE_2D, texture);
                }
                

                uint32_t getId() const noexcept override;

                int getWidth() const noexcept override;
                int getHeight() const noexcept override;

            private:
                GLuint texture;
                uint32_t width;
                uint32_t height;
        };
    };
};

#endif