#include "OpenGLTexture.hpp"

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

            uint32_t width;
            uint32_t height;
            std::vector<uint8_t> data;
            lodepng::decode(data, width, height, path.string());

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        OpenGlTexture::~OpenGlTexture() noexcept
        {
            glDeleteTextures(1, &texture);
        }

        void OpenGlTexture::bind(const uint32_t slotId) const noexcept
        {
            glActiveTexture(GL_TEXTURE0 + slotId);
            glBindTexture(GL_TEXTURE_2D, texture);
        }
    };
};