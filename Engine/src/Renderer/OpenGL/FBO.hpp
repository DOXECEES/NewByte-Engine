#ifndef SRC_RENDERER_OPENGL_FBO_HPP
#define SRC_RENDERER_OPENGL_FBO_HPP

#include "../../Core.hpp"
#include "Renderer/IFrameBuffer.hpp"

#include <glad/glad.h>


#include <vector>
#include <string_view>

#include <limits>

namespace nb
{
    namespace OpenGl
    {
        class FBO : public Renderer::IFrameBuffer
        {
        
        public:

            enum class TextureType
            {
                COLOR           = 0,
                DEPTH           = 1,
                STENCIL         = 2,
                DEPTH_STENCIL   = 3,
            };

            enum class RenderBufferType
            {
                DEPTH_ONLY           = 0,
                STENCIL_ONLY         = 1,
                DEPTH_STENCIL        = 2,
            };

            FBO() noexcept;
            virtual ~FBO() noexcept;

            FBO(const FBO& other) noexcept = delete;
            FBO& operator=(const FBO& other) noexcept = delete;

            FBO(FBO&& other) noexcept = delete;
            FBO& operator=(FBO&& other) noexcept = delete;

            void bind() noexcept override;
            void unBind() noexcept override;

            void bindTexture(TextureType type) noexcept;
            void bindRenderBuffer(RenderBufferType renderBufferType) noexcept;

            void setSize(const GLuint width, const GLuint height) noexcept;

            void addRenderBufferAttachment(RenderBufferAttachment attachment) noexcept override;
            void addTextureAttachment(TextureAttachment attachment) noexcept override;
            bool finalize() noexcept override;

            
            NB_NODISCARD bool checkIsSizeValid() const noexcept;


            NB_NODISCARD static GLint getMaxCountOfColorAttachments() noexcept;


            NB_NODISCARD inline bool getIsDepthBufferAttached() const noexcept           { return isDepthBufferAttached; }
            NB_NODISCARD inline bool getIsStencilBufferAttached() const noexcept         { return isStencilBufferAttached; }
            NB_NODISCARD inline GLuint getRenderBuffer() const noexcept                  { return renderBuffer; }
            NB_NODISCARD inline const std::vector<GLuint>& getTextures() const noexcept  { return textures; }
            NB_NODISCARD inline uint32 getTexture(uint8 index) const noexcept override   { return static_cast<uint32>(textures[index]); }

            NB_NODISCARD inline uint8_t getColorTextureCount() const noexcept            { return colorTextureCount; }
            NB_NODISCARD inline GLuint getWidth() const noexcept                         { return width; }
            NB_NODISCARD inline GLuint getHeight() const noexcept                        { return height; }

        private:
            void setupTextureParams() const noexcept;
            void errorMessage(std::string_view message) const noexcept;

        private:
            inline static constexpr GLuint GL_UINT_MAX = 0xFFFFFFFF;
            GLuint buffer                               = 0;

            bool                isDepthBufferAttached   = false;
            bool                isStencilBufferAttached = false;
            GLuint              renderBuffer            = 0;
            std::vector<GLuint> textures;
            uint8_t             colorTextureCount       = 0;
            GLuint              width                   = GL_UINT_MAX;  
            GLuint              height                  = GL_UINT_MAX;   
        };
    };

};

#endif