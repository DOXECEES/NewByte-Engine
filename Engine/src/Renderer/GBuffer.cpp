#include "GBuffer.hpp"
#include "RendererTracker.hpp"

namespace nb::Renderer
{
    GBuffer::GBuffer(
        nbstl::NonOwningPtr<IRenderAPI> api,
        uint32                          width,
        uint32                          height
    ) noexcept
    {
        framebuffer = api->createFrameBuffer(width, height);
        framebuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::COLOR_HDR, "Normals");
        framebuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::COLOR_HDR, "View position");
        framebuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::COLOR_HDR, "Albedo");
        framebuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::COLOR_HDR, "Orm");
        framebuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::DEPTH, "Depth");
        framebuffer->finalize();

        RendererTracker::addFrameBuffer("GBuffer", framebuffer);
        
        framebuffer->setDrawBuffers(4);
    }

    const Ref<IFrameBuffer>& GBuffer::getFramebuffer() const noexcept
    {
        return framebuffer;
    }
};