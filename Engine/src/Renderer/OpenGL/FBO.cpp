#include "FBO.hpp"

namespace nb
{
    namespace OpenGl
    {
        FBO::FBO() noexcept
            :IBuffer()
        {
            glGenFramebuffers(1, &buffer);
        }

        FBO::~FBO() noexcept
        {
            glDeleteFramebuffers(1, &buffer);
        }

        void FBO::bind() const noexcept
        {
            glBindFramebuffer(GL_FRAMEBUFFER, buffer);
        }

        void FBO::unBind() const noexcept
        {
            
        }
    };
};
