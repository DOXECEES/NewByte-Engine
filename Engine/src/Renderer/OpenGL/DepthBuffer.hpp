#ifndef SRC_RENDERER_OPENGL_DEPTHBUFFER_HPP
#define SRC_RENDERER_OPENGL_DEPTHBUFFER_HPP

#include "IBuffer.hpp"

namespace nb
{
    namespace OpenGl
    {
        class DepthBuffer : public IBuffer
        {
        public:
            DepthBuffer(const GLuint width = 1024, const GLuint height = 1024) noexcept;
            ~DepthBuffer() = default;

            void bind() const noexcept override;
            void unBind() const noexcept override;

            GLuint generateDepthMap(const GLuint width, const GLuint height) const noexcept;

            GLuint getDepthMap() const noexcept;

            GLuint getWidth() const noexcept;
            GLuint getHeight() const noexcept;

        private:
            GLuint colorTexture = 0;
            GLuint width        = 1024;
            GLuint height       = 1024;
        };
    };
};

#endif