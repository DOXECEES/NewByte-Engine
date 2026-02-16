#ifndef SRC_RENDERER_IFRAMEBUFFER_HPP
#define SRC_RENDERER_IFRAMEBUFFER_HPP
#include <Types.hpp>

namespace nb::Renderer
{
    class IFrameBuffer
    {
    public:

        enum class RenderBufferAttachment
        {
            COLOR,
            DEPTH,
            STENCIL,
            DEPTH_STENCIL,
        };

        enum class TextureAttachment
        {
            COLOR,
            DEPTH,
            STENCIL,
            DEPTH_STENCIL,
        };

        virtual ~IFrameBuffer() = default;
        virtual void bind() noexcept = 0;
        virtual void unBind() noexcept = 0;
        virtual uint32 getWidth() const noexcept = 0;
        virtual uint32 getHeight() const noexcept = 0;
        virtual bool finalize() noexcept = 0;

        virtual uint32 getTexture(uint8 index = 0) const noexcept = 0;
        virtual void addRenderBufferAttachment(RenderBufferAttachment attachment) noexcept = 0;
        virtual void addTextureAttachment(TextureAttachment attachment) noexcept = 0;

    };
}

#endif