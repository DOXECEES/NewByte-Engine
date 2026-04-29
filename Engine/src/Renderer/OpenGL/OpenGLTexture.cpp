// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "OpenGLTexture.hpp"

#include "../../dependencies/stb/stb_image.h"
namespace nb
{
    namespace OpenGl
    {
        OpenGlTexture::OpenGlTexture(const std::filesystem::path& path) noexcept
        {
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


            stbi_set_flip_vertically_on_load(true);

            int channels;
            int            tempWidth = 0;
            int            tempHeight = 0;
            unsigned char* data = stbi_load(
                path.string().c_str(), &tempWidth, &tempHeight, &channels, STBI_rgb_alpha
            );

            
            if (data)
            {
                width = tempWidth;
                height = tempHeight;
                glTexImage2D(
                    GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
                );
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else
            {
                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::FATAL, "Failed to load texture from file!")
                    .with("Path", path);
            }

            stbi_image_free(data);

        }
        OpenGlTexture::OpenGlTexture(
            int width,
            int height,
            GLint internalFormat,
            GLenum dataFormat,
            GLenum dataType,
            void* data
        ) noexcept
        {
            this->width = width;
            this->height = height;

            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D(
                GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, dataType, data
            );

            glGenerateMipmap(GL_TEXTURE_2D);
        }

        OpenGlTexture::~OpenGlTexture() noexcept
        {
            glDeleteTextures(1, &texture);
        }

        //void OpenGlTexture::bind(const uint32_t slotId)
        //{
        //    glActiveTexture(GL_TEXTURE0 + slotId);
        //    glBindTexture(GL_TEXTURE_2D, texture);
        //}

        uint32_t OpenGlTexture::getId() const noexcept
        {
            return texture;
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