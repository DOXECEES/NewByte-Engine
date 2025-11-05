// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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

            unBind();
            for (auto& i : textures)
                glDeleteTextures(1, &i);
            glDeleteRenderbuffers(1, &renderBuffer);
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
                errorMessage("Framebuffer size is not valid!");
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
                if(colorTextureCount > getMaxCountOfColorAttachments())
                {
                    errorMessage("Max count of color attachments reached!");
                    return;
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
                    errorMessage("Depth buffer already attached!");
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
                    errorMessage("Stencil buffer already attached!");
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
                    errorMessage("Depth or stencil buffer already attached!");
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
                errorMessage("Framebuffer size is not valid!");
            }

            if (isDepthBufferAttached || isStencilBufferAttached)
            {
                errorMessage("Depth or stencil buffer already attached!");
            }

            glGenRenderbuffers(1, &renderBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
         
            switch (renderBufferType)
            {
            case RenderBufferType::DEPTH_ONLY:
            {
                if(isDepthBufferAttached)
                {
                    errorMessage("Cannot attach renderbuffer. Depth buffer already attached!");
                }

                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);
            
                break;
            }

            case RenderBufferType::STENCIL_ONLY:
            {
                if(isStencilBufferAttached)
                {
                    errorMessage("Cannot attach renderbuffer. Stencil buffer already attached!");
                }
                glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX, width, height);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);
                break;
            }
            case RenderBufferType::DEPTH_STENCIL:
            {
                if(isDepthBufferAttached || isStencilBufferAttached)
                {
                    errorMessage("Cannot attach renderbuffer. Depth or stencil buffer already attached!");
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
            constexpr GLuint max = std::numeric_limits<GLuint>::max();

            return this->width != max || this->height != max;
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

        void FBO::errorMessage(std::string_view message) const noexcept
        {
            Debug::debug(message);
            abort();
        }
    };
};
