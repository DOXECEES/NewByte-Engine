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
                Debug::debug("Framebuffer is not complete!");
        
            this->unBind();
        }
    };
};