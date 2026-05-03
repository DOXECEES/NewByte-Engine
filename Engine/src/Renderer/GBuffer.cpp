#include "GBuffer.hpp"

namespace nb::Renderer
{
    GBuffer::GBuffer(
        nbstl::NonOwningPtr<IRenderAPI> api,
        uint32                          width,
        uint32                          height
    ) noexcept
    {
        framebuffer = api->createFrameBuffer(width, height);
        framebuffer->addRenderBufferAttachment(IFrameBuffer::RenderBufferAttachment::DEPTH);
        framebuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::COLOR_HDR);
        framebuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::COLOR_HDR);
        framebuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::COLOR);
        framebuffer->finalize();
    }

    const Ref<IFrameBuffer>& GBuffer::getFramebuffer() const noexcept
    {
        return framebuffer;
    }
};