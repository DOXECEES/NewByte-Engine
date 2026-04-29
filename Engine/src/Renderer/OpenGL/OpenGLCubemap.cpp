#include "OpenGLCubemap.hpp"
#include <algorithm>
#include <cmath>
#include <glad/glad.h>

namespace nb::OpenGl
{
    static GLsizei calculateMipLevels(uint32_t size)
    {
        return static_cast<GLsizei>(std::floor(std::log2(size))) + 1;
    }

    OpenGLCubemap::OpenGLCubemap() noexcept
        : handle(0)
        , size(0)
    {
        glGenTextures(1, &envCubemap);
    }

    OpenGLCubemap::OpenGLCubemap(
        uint32_t size,
        int      internalFormat
    ) noexcept
        : handle(0)
        , size(size)
    {
        glGenTextures(1, &envCubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

        GLsizei levels = calculateMipLevels(size);
        glTexStorage2D(GL_TEXTURE_CUBE_MAP, levels, internalFormat, size, size);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    OpenGLCubemap::~OpenGLCubemap()
    {
        if (handle != 0)
        {
            glMakeTextureHandleNonResidentARB(handle);
        }
        if (envCubemap != 0)
        {
            glDeleteTextures(1, &envCubemap);
        }
    }

    void OpenGLCubemap::finalizeBindless() noexcept
    {
        if (handle != 0)
        {
            return;
        }

        handle = glGetTextureHandleARB(envCubemap);
        glMakeTextureHandleResidentARB(handle);
    }

    uint64_t OpenGLCubemap::getHandle() const noexcept
    {
        return handle;
    }

    uint32_t OpenGLCubemap::getId() const noexcept
    {
        return envCubemap;
    }

    void OpenGLCubemap::bind(uint32_t slot) const noexcept
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    }

    void OpenGLCubemap::unbind() const noexcept
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

} 