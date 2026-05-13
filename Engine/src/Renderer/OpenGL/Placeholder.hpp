#ifndef SRC_RENDERER_OPENGL_PLACEHOLDER_HPP
#define SRC_RENDERER_OPENGL_PLACEHOLDER_HPP

#include <glad/glad.h>

namespace nb::OpenGl
{
    GLuint64 createPlaceholderForEmission() noexcept;
    GLuint64 createPlaceholderForDepth() noexcept;
    GLuint64 createPlaceholderForNoise() noexcept;
}


#endif
