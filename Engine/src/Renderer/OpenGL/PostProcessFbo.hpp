#ifndef SRC_RENDERER_OPENGL_POSTPROCESSFBO_HPP
#define SRC_RENDERER_OPENGL_POSTPROCESSFBO_HPP

#include <glad/glad.h>

#include "FBO.hpp"

namespace nb
{
    namespace OpenGl
    {
        class PostProcessFbo : public FBO
        {
        public:
            PostProcessFbo(const GLuint width = 1024, const GLuint height = 1024);
            ~PostProcessFbo() = default;

            GLuint getQuadTexture() const noexcept { return this->getTextures()[0]; };
        };

        
    };
};

#endif