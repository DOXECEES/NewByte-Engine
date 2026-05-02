#ifndef SRC_RENDERER_OPENGL_OPENGLUTILS_HPP
#define SRC_RENDERER_OPENGL_OPENGLUTILS_HPP

#include <NbCore.hpp>
#include <glad/glad.h>

#include "Error/ErrorManager.hpp"
#include "Renderer/Cubemap.hpp"


namespace nb::OpenGl::OpenGlUtils
{

    NB_NODISCARD GLint toFormat(Renderer::CubemapParameters::Format format) noexcept
    {
        switch (format)
        {
        case Renderer::CubemapParameters::Format::Rgba8UNormal:
            return GL_RGBA8;
        case Renderer::CubemapParameters::Format::Rgba16Float:
            return GL_RGBA16F;
        case Renderer::CubemapParameters::Format::Rgba32Float:
            return GL_RGBA32F;
        case Renderer::CubemapParameters::Format::R32Float:
            return GL_R32F;
        default:
        {
            nb::Error::ErrorManager::instance().report(
                nb::Error::Type::WARNING, "Unsupported internal format"
            );
            return 0;
        }
        }
    }

    NB_NODISCARD GLint toWrap(Renderer::CubemapParameters::Wrapping wrap) noexcept
    {
        switch (wrap)
        {
        case Renderer::CubemapParameters::Wrapping::Repeat:
            return GL_REPEAT;
        case Renderer::CubemapParameters::Wrapping::MirroredRepeat:
            return GL_MIRRORED_REPEAT;
        case Renderer::CubemapParameters::Wrapping::ClampToEdge:
            return GL_CLAMP_TO_EDGE;
        case Renderer::CubemapParameters::Wrapping::ClampToBorder:
            return GL_CLAMP_TO_BORDER;
        default:
            nb::Error::ErrorManager::instance().report(
                nb::Error::Type::WARNING, "Unsupported wrap mode"
            );
            return GL_CLAMP_TO_EDGE; 
        }
    }

    NB_NODISCARD GLint toMagFilter(Renderer::CubemapParameters::Filtering filter) noexcept
    {
        switch (filter)
        {
        case Renderer::CubemapParameters::Filtering::Nearest:
            return GL_NEAREST;
        case Renderer::CubemapParameters::Filtering::Linear:
            return GL_LINEAR;
        default:
            return GL_LINEAR;
        }
    }

    NB_NODISCARD GLint toMinFilter(
        Renderer::CubemapParameters::Filtering filter,
        bool                                   hasMipmaps
    ) noexcept
    {
        if (hasMipmaps)
        {
            switch (filter)
            {
            case Renderer::CubemapParameters::Filtering::Nearest:
                return GL_NEAREST_MIPMAP_NEAREST;
            case Renderer::CubemapParameters::Filtering::Linear:
                return GL_LINEAR_MIPMAP_LINEAR;
            default:
                return GL_LINEAR_MIPMAP_LINEAR;
            }
        }
        else
        {
            switch (filter)
            {
            case Renderer::CubemapParameters::Filtering::Nearest:
                return GL_NEAREST;
            case Renderer::CubemapParameters::Filtering::Linear:
                return GL_LINEAR;
            default:
                return GL_LINEAR;
            }
        }
    }
}


#endif