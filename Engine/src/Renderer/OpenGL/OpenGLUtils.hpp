#ifndef SRC_RENDERER_OPENGL_OPENGLUTILS_HPP
#define SRC_RENDERER_OPENGL_OPENGLUTILS_HPP

#include <NbCore.hpp>
#include <glad/glad.h>

#include "Error/ErrorManager.hpp"

#include "Renderer/TextureParameters.hpp"


namespace nb::OpenGl::OpenGlUtils
{

    inline NB_NODISCARD GLint toFormat(Renderer::Format format) noexcept
    {
        switch (format)
        {
        case Renderer::Format::Rgba8UNormal:
            return GL_RGBA8;
        case Renderer::Format::Rgba16Float:
            return GL_RGBA16F;
        case Renderer::Format::Rgba32Float:
            return GL_RGBA32F;
        case Renderer::Format::R32Float:
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

    inline NB_NODISCARD GLint toWrap(Renderer::Wrapping wrap) noexcept
    {
        switch (wrap)
        {
        case Renderer::Wrapping::Repeat:
            return GL_REPEAT;
        case Renderer::Wrapping::MirroredRepeat:
            return GL_MIRRORED_REPEAT;
        case Renderer::Wrapping::ClampToEdge:
            return GL_CLAMP_TO_EDGE;
        case Renderer::Wrapping::ClampToBorder:
            return GL_CLAMP_TO_BORDER;
        default:
            nb::Error::ErrorManager::instance().report(
                nb::Error::Type::WARNING, "Unsupported wrap mode"
            );
            return GL_CLAMP_TO_EDGE; 
        }
    }

    inline NB_NODISCARD GLint toMagFilter(Renderer::Filtering filter) noexcept
    {
        switch (filter)
        {
        case Renderer::Filtering::Nearest:
            return GL_NEAREST;
        case Renderer::Filtering::Linear:
            return GL_LINEAR;
        default:
            return GL_LINEAR;
        }
    }

    inline NB_NODISCARD GLint toMinFilter(
        Renderer::Filtering filter,
        bool                                   hasMipmaps
    ) noexcept
    {
        if (hasMipmaps)
        {
            switch (filter)
            {
            case Renderer::Filtering::Nearest:
                return GL_NEAREST_MIPMAP_NEAREST;
            case Renderer::Filtering::Linear:
                return GL_LINEAR_MIPMAP_LINEAR;
            default:
                return GL_LINEAR_MIPMAP_LINEAR;
            }
        }
        else
        {
            switch (filter)
            {
            case Renderer::Filtering::Nearest:
                return GL_NEAREST;
            case Renderer::Filtering::Linear:
                return GL_LINEAR;
            default:
                return GL_LINEAR;
            }
        }
    }
}


#endif