// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "DepthBuffer.hpp"

#include "../../Debug.hpp"

namespace nb
{
    namespace OpenGl
    {
        DepthBuffer::DepthBuffer(const GLuint width, const GLuint height) noexcept
        {
            this->setSize(width, height);
            this->bindTexture(TextureType::DEPTH);
            if (!this->finalize())
                Debug::debug("Framebuffer is not complete!");
        
            this->unBind();

        }

        GLuint DepthBuffer::generateDepthMap(const GLuint width, const GLuint height) const noexcept
        {
            return 0;
        }
        
        GLuint DepthBuffer::getDepthMap() const noexcept
        {
            return this->getTextures()[0];
        }
    };
};

