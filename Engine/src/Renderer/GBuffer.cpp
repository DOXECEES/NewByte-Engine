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
        framebuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::COLOR_HDR);
        framebuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::COLOR_HDR);
        framebuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::DEPTH);
        framebuffer->finalize();

        framebuffer->setDrawBuffers(2);
    }

    const Ref<IFrameBuffer>& GBuffer::getFramebuffer() const noexcept
    {
        return framebuffer;
    }
};