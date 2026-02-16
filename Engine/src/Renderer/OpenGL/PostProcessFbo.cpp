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
        
            this->unBind();
        }

        PostProcessFbo::~PostProcessFbo()
        {
        }
    };
};