#ifndef SRC_RENDERER_OPENGL_DEPTHBUFFER_HPP
#define SRC_RENDERER_OPENGL_DEPTHBUFFER_HPP

#include "FBO.hpp"

namespace nb
{
    namespace OpenGl
    {
        class DepthBuffer : public FBO
        {
        public:
            DepthBuffer(const GLuint width = 1024, const GLuint height = 1024) noexcept;
            virtual ~DepthBuffer() = default;

            GLuint generateDepthMap(const GLuint width, const GLuint height) const noexcept;

            GLuint getDepthMap() const noexcept;


        private:
        };
    };
};

#endif