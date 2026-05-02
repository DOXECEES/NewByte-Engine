#include "OpenGLCubemap.hpp"
#include <algorithm>
#include <cmath>
#include <glad/glad.h>
#include "OpenGLUtils.hpp"

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
        finalizeBindless();
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
    
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    }

    OpenGLCubemap::OpenGLCubemap(const Renderer::CubemapParameters& params) noexcept
    {
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &envCubemap);

        GLsizei levels = calculateMipLevels(params.size);
        glTextureStorage2D(
            envCubemap, levels, OpenGlUtils::toFormat(params.format), params.size, params.size
        );

        glTextureParameteri(
            envCubemap, GL_TEXTURE_MIN_FILTER, OpenGlUtils::toMinFilter(params.minFilter, params.generateMipmaps)
        );
        glTextureParameteri(
            envCubemap, GL_TEXTURE_MAG_FILTER, OpenGlUtils::toMagFilter(params.magFilter)
        );

        
        glTextureParameteri(envCubemap, GL_TEXTURE_WRAP_S, OpenGlUtils::toWrap(params.wrapU));
        glTextureParameteri(envCubemap, GL_TEXTURE_WRAP_T, OpenGlUtils::toWrap(params.wrapV));
        glTextureParameteri(envCubemap, GL_TEXTURE_WRAP_R, OpenGlUtils::toWrap(params.wrapW));
        glTextureParameteri(envCubemap, GL_TEXTURE_COMPARE_MODE, GL_NONE);

        if (!params.generateMipmaps)
        {
            glTextureParameteri(envCubemap, GL_TEXTURE_BASE_LEVEL, 0);
            glTextureParameteri(envCubemap, GL_TEXTURE_MAX_LEVEL, 0);
        }

        finalizeBindless();
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