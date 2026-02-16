// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "FBO.hpp"
#include "Error/ErrorManager.hpp"
#include "../../Debug.hpp"

namespace nb
{
    namespace OpenGl
    {
        FBO::FBO() noexcept
            : Renderer::IFrameBuffer()
        {
            glGenFramebuffers(1, &buffer);
        }

        FBO::~FBO() noexcept
        {
            unBind();
            if (!textures.empty())
                glDeleteTextures(static_cast<GLsizei>(textures.size()), textures.data());
            if (renderBuffer)
                glDeleteRenderbuffers(1, &renderBuffer);
            glDeleteFramebuffers(1, &buffer);
        }

        void FBO::bind() noexcept
        {
            glBindFramebuffer(GL_FRAMEBUFFER, buffer);
        }

        void FBO::unBind() noexcept
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void FBO::bindTexture(TextureType type) noexcept
        {
            if (!checkIsSizeValid())
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
                if (colorTextureCount >= getMaxCountOfColorAttachments())
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
                if (isDepthBufferAttached)
                {
                    errorMessage("Depth buffer already attached!");
                }

                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
                glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);

                if (colorTextureCount == 0) {
                    glDrawBuffer(GL_NONE);
                    glReadBuffer(GL_NONE);
                }

                isDepthBufferAttached = true; 
                break;
            }
            case TextureType::STENCIL:
            {
                if (isStencilBufferAttached) { errorMessage("Stencil already attached!"); }
                glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX8, width, height, 0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, nullptr);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
                isStencilBufferAttached = true;
                break;
            }
            case TextureType::DEPTH_STENCIL:
            {
                if (isDepthBufferAttached || isStencilBufferAttached) { errorMessage("Conflict!"); }
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
                isDepthBufferAttached = true;
                isStencilBufferAttached = true;
                break;
            }
            default: break;
            }
            glBindTexture(GL_TEXTURE_2D, 0); 
        }

        void FBO::bindRenderBuffer(RenderBufferType renderBufferType) noexcept
        {
            if (!checkIsSizeValid()) { errorMessage("Size is not valid!"); }

            glBindFramebuffer(GL_FRAMEBUFFER, buffer);
            glGenRenderbuffers(1, &renderBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);

            switch (renderBufferType)
            {
            case RenderBufferType::DEPTH_ONLY:
            {
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);
                isDepthBufferAttached = true; 
                break;
            }
            case RenderBufferType::STENCIL_ONLY:
            {
                glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);
                isStencilBufferAttached = true;
                break;
            }
            case RenderBufferType::DEPTH_STENCIL:
            {
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);
                isDepthBufferAttached = true;
                isStencilBufferAttached = true;
                break;
            }
            default: break;
            }

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                errorMessage("Framebuffer RenderBuffer finalize failed!");
            }
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }

        bool FBO::finalize() noexcept
        {
            bind();
            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (status != GL_FRAMEBUFFER_COMPLETE)
            {
                nb::Error::ErrorManager::instance()
                    .report(Error::Type::FATAL, "Failed to finalize framebuffer")
                    .with("Status", (int)status);
                return false;
            }
            unBind();
            return true;
        }

        void FBO::setSize(const GLuint width, const GLuint height) noexcept
        {
            this->width  = width;
            this->height = height;
        }

        void FBO::addRenderBufferAttachment(RenderBufferAttachment attachment) noexcept
        {
            switch (attachment)
            {
            case nb::Renderer::IFrameBuffer::RenderBufferAttachment::COLOR:
            {
                break;
            }
            case nb::Renderer::IFrameBuffer::RenderBufferAttachment::DEPTH:
            {  
                bindRenderBuffer(RenderBufferType::DEPTH_ONLY);
                break;
            }
            case nb::Renderer::IFrameBuffer::RenderBufferAttachment::STENCIL:
            {
                bindRenderBuffer(RenderBufferType::STENCIL_ONLY);
                break;
            }
            case nb::Renderer::IFrameBuffer::RenderBufferAttachment::DEPTH_STENCIL:
            {
                bindRenderBuffer(RenderBufferType::DEPTH_STENCIL);
                break;
            }
            default:
                break;
            }
        }

        void FBO::addTextureAttachment(TextureAttachment attachment) noexcept
        {
            switch (attachment)
            {
            case nb::Renderer::IFrameBuffer::TextureAttachment::COLOR:
            {
                bindTexture(TextureType::COLOR);
                break;
            }
            case nb::Renderer::IFrameBuffer::TextureAttachment::DEPTH:
            {
                bindTexture(TextureType::DEPTH);
                break;
            }
            case nb::Renderer::IFrameBuffer::TextureAttachment::STENCIL:
            {
                bindTexture(TextureType::STENCIL);
                break;
            }
            case nb::Renderer::IFrameBuffer::TextureAttachment::DEPTH_STENCIL:
            {
                bindTexture(TextureType::DEPTH_STENCIL);
                break;
            }
            default:
                break;
            }
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
