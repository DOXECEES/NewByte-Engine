#ifndef SRC_RENDERER_IFRAMEBUFFER_HPP
#define SRC_RENDERER_IFRAMEBUFFER_HPP
#include <Types.hpp>

#include <string_view>
#include <string>


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
            COLOR_HDR,
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
        virtual uint64_t getTextureHandle(uint8 index = 0) const noexcept = 0;
        virtual uint64_t getTextureByName(std::string_view name) const noexcept = 0;
        virtual const std::string& getNameByTextureIndex(uint8 index) const noexcept = 0;

        virtual size_t getTextureCount() const noexcept = 0;
        virtual TextureAttachment getTextureAttachmentType(uint8 index = 0) const noexcept = 0;

        virtual void addRenderBufferAttachment(RenderBufferAttachment attachment) noexcept = 0;
        virtual void addTextureAttachment(TextureAttachment attachment, std::string_view name) noexcept = 0;

        virtual void setDrawBuffers(uint8 count) noexcept = 0;


    };
}

#endif