// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "PostProcessFbo.hpp"

#include "../../Debug.hpp"

namespace nb
{
    namespace OpenGl
    {
        PostProcessFbo::PostProcessFbo(const GLuint width, const GLuint height)
        {
            this->setSize(width, height);
            this->bindTexture(TextureType::COLOR);
            this->bindRenderBuffer(RenderBufferType::DEPTH_STENCIL);

            if (!this->finalize())
            {
                Debug::debug("Framebuffer is not complete!");
                int errorCode = glCheckFramebufferStatus(GL_FRAMEBUFFER);
                Debug::debug(errorCode);
            }
        
            this->unBind();
        }

        PostProcessFbo::~PostProcessFbo()
        {
        }
    };
};