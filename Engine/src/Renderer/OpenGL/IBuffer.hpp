#ifndef SRC_RENDERER_OPENGL_IBUFFER_HPP
#define SRC_RENDERER_OPENGL_IBUFFER_HPP

#include <glad/glad.h>

namespace nb
{
    namespace OpenGl
    {
        class IBuffer
        {
            public:
                IBuffer() = default;
                virtual ~IBuffer() = default;
                virtual void bind() const noexcept = 0;
                virtual void unBind() const noexcept = 0;

                

            protected:
                GLuint buffer;
        };
    };
};


#endif