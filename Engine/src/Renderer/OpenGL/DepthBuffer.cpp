#include "DepthBuffer.hpp"

#include "../../Debug.hpp"

namespace nb
{
    namespace OpenGl
    {
        DepthBuffer::DepthBuffer(const GLuint width, const GLuint height) noexcept
            : width(width)
            , height(height)
        {
            glGenFramebuffers(1, &buffer);
            glBindFramebuffer(GL_FRAMEBUFFER, buffer);
        
            // Цветовая текстура
            glGenTextures(1, &colorTexture);
            glBindTexture(GL_TEXTURE_2D, colorTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
        
            // Depth + stencil через Renderbuffer
            GLuint rbo;
            glGenRenderbuffers(1, &rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
        
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                Debug::debug("Framebuffer is not complete!");
        
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

        }

        void DepthBuffer::bind() const noexcept
        {
            glBindFramebuffer(GL_FRAMEBUFFER, buffer);
            //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
        }

        void DepthBuffer::unBind() const noexcept
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        GLuint DepthBuffer::generateDepthMap(const GLuint width, const GLuint height) const noexcept
        {
            return 0;
        }
        
        GLuint DepthBuffer::getDepthMap() const noexcept
        {
            return colorTexture;
        }
        GLuint DepthBuffer::getWidth() const noexcept
        {
            return width;
        }
        GLuint DepthBuffer::getHeight() const noexcept
        {
            return height;
        }
    };
};

