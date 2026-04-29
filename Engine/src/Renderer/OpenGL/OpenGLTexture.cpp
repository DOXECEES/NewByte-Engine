#include "OpenGLTexture.hpp"
#include "../../dependencies/stb/stb_image.h"
#include <cmath>

namespace nb
{
    namespace OpenGl
    {
        static GLsizei calculateMipLevels(
            int w,
            int h
        )
        {
            return static_cast<GLsizei>(std::floor(std::log2(std::max(w, h)))) + 1;
        }

        // Конструктор для загрузки из файла (обычно статичные текстуры)
        OpenGlTexture::OpenGlTexture(const std::filesystem::path& path) noexcept
            : handle(0)
            , texture(0)
            , width(0)
            , height(0)
        {
            stbi_set_flip_vertically_on_load(true);

            int            channels;
            int            tempWidth  = 0;
            int            tempHeight = 0;
            unsigned char* data       = stbi_load(
                path.string().c_str(), &tempWidth, &tempHeight, &channels, STBI_rgb_alpha
            );

            if (data)
            {
                width  = tempWidth;
                height = tempHeight;

                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                GLsizei levels = calculateMipLevels(width, height);
                glTexStorage2D(GL_TEXTURE_2D, levels, GL_RGBA8, width, height);

                glTexSubImage2D(
                    GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data
                );
                glGenerateMipmap(GL_TEXTURE_2D);

                stbi_image_free(data);

                finalizeBindless();
            }
            else
            {
                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::FATAL, "Failed to load texture from file!")
                    .with("Path", path);
            }
        }

        OpenGlTexture::OpenGlTexture(
            int    width,
            int    height,
            GLint  internalFormat,
            GLenum dataFormat,
            GLenum dataType,
            void*  data
        ) noexcept
            : handle(0)
            , texture(0)
            , width(width)
            , height(height)
        {
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            GLsizei levels = data ? calculateMipLevels(width, height) : 1;
            glTexStorage2D(GL_TEXTURE_2D, levels, internalFormat, width, height);

            if (data)
            {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, dataFormat, dataType, data);
                glGenerateMipmap(GL_TEXTURE_2D);

                finalizeBindless();
            }
        }

        OpenGlTexture::~OpenGlTexture() noexcept
        {
            if (handle != 0)
            {
                glMakeTextureHandleNonResidentARB(handle);
            }
            if (texture != 0)
            {
                glDeleteTextures(1, &texture);
            }
        }

        void OpenGlTexture::finalizeBindless() noexcept
        {
            if (handle != 0)
            {
                return; 
            }

            handle = glGetTextureHandleARB(texture);
            glMakeTextureHandleResidentARB(handle);
        }

        uint32_t OpenGlTexture::getId() const noexcept
        {
            return texture;
        }
        uint64_t OpenGlTexture::getHandle() const noexcept
        {
            return handle;
        }
        int OpenGlTexture::getWidth() const noexcept
        {
            return width;
        }
        int OpenGlTexture::getHeight() const noexcept
        {
            return height;
        }
    }; 
};