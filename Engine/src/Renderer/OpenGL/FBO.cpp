#include "FBO.hpp"

#include "../../Debug.hpp"

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
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        void FBO::bindTexture(TextureType type) noexcept
        {
            if(!checkIsSizeValid())
            {
                Debug::debug("Framebuffer size is not valid!");
                abort();
            }

            glBindFramebuffer(GL_FRAMEBUFFER, buffer);
            textures.push_back(0);

            GLuint& texture = textures.back();

            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            switch (type)
            {
            case TextureType::COLOR:
            {
                // temp abort 
                if(colorTextureCount > getMaxCountOfColorAttachments())
                {
                    Debug::debug("Max count of color attachments reached!");
                    abort();
                }

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
                setupTextureParams();
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorTextureCount, GL_TEXTURE_2D, texture, 0);
        
                colorTextureCount++;
                break;
            }
            case TextureType::DEPTH:
            {
                if(isDepthBufferAttached)
                {
                    Debug::debug("Depth buffer already attached!");
                    abort();
                }

                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
                setupTextureParams();
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
                break;
            }
            case TextureType::STENCIL:
            {
                if(isStencilBufferAttached)
                {
                    Debug::debug("Stencil buffer already attached!");
                    abort();
                }

                glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX, width, height, 0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, nullptr);
                setupTextureParams();
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
                break;
            }
            case TextureType::DEPTH_STENCIL:
            {
                if(isDepthBufferAttached || isStencilBufferAttached)
                {
                    Debug::debug("Depth or stencil buffer already attached!");
                    abort();
                }
                
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_BYTE, nullptr);
                setupTextureParams();
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
             
                break;
            }
            
            default:
                break;
            }            
        }

        void FBO::bindRenderBuffer(RenderBufferType renderBufferType) noexcept
        {
            if(!checkIsSizeValid())
            {
                Debug::debug("Framebuffer size is not valid!");
                abort();
            }

            if (isDepthBufferAttached || isStencilBufferAttached)
            {
                Debug::debug("Depth or stencil buffer already attached!");
                abort();
            }

            glGenRenderbuffers(1, &renderBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
         
            switch (renderBufferType)
            {
            case RenderBufferType::DEPTH_ONLY:
            {
                if(isDepthBufferAttached)
                {
                    Debug::debug("Cannot attach renderbuffer. Depth buffer already attached!");
                    abort();
                }

                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);
            
                break;
            }

            case RenderBufferType::STENCIL_ONLY:
            {
                if(isStencilBufferAttached)
                {
                    Debug::debug("Cannot attach renderbuffer. Stencil buffer already attached!");
                    abort();
                }
                glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX, width, height);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);
                break;
            }
            case RenderBufferType::DEPTH_STENCIL:
            {
                if(isDepthBufferAttached || isStencilBufferAttached)
                {
                    Debug::debug("Cannot attach renderbuffer. Depth or stencil buffer already attached!");
                    abort();
                }
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);
                break;
            }
            default:
                break;
            }
        }

        void FBO::setSize(const GLuint width, const GLuint height) noexcept
        {
            this->width  = width;
            this->height = height;
        }

        bool FBO::checkIsSizeValid() const noexcept
        {
            return this->width != 0xFFFFFFFF 
                || this->height != 0xFFFFFFFF;
        }

        GLint FBO::getMaxCountOfColorAttachments() noexcept
        {
            static GLint maxColorAttachments = -1;
            if (maxColorAttachments == -1)
            {
                glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
            }
            return maxColorAttachments;
        }
        bool FBO::finalize() noexcept
        {
            return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
        }
        void FBO::setupTextureParams() const noexcept
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
    };
};
