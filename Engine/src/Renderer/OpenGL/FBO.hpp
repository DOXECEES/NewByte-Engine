#ifndef SRC_RENDERER_OPENGL_FBO_HPP
#define SRC_RENDERER_OPENGL_FBO_HPP

#include "IBuffer.hpp"

#include <vector>
#include <limits>

namespace nb
{
    namespace OpenGl
    {
        class FBO : public IBuffer 
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
            ~FBO() noexcept;

            FBO(const FBO& other) noexcept = delete;
            FBO& operator=(const FBO& other) noexcept = delete;

            FBO(FBO&& other) noexcept = delete;
            FBO& operator=(FBO&& other) noexcept = delete;

            virtual void bind() const noexcept override;
            virtual void unBind() const noexcept override;

            void bindTexture(TextureType type) noexcept;
            void bindRenderBuffer(RenderBufferType renderBufferType) noexcept;

            void setSize(const GLuint width, const GLuint height) noexcept;
            
            bool checkIsSizeValid() const noexcept;



            bool finalize() noexcept;

            static GLint getMaxCountOfColorAttachments() noexcept;


            inline bool getIsDepthBufferAttached() const noexcept           { return isDepthBufferAttached; }
            inline bool getIsStencilBufferAttached() const noexcept         { return isStencilBufferAttached; }
            inline GLuint getRenderBuffer() const noexcept                  { return renderBuffer; }
            inline const std::vector<GLuint>& getTextures() const noexcept  { return textures; }
            inline uint8_t getColorTextureCount() const noexcept            { return colorTextureCount; }
            inline GLuint getWidth() const noexcept                         { return width; }
            inline GLuint getHeight() const noexcept                        { return height; }

        private:
            void setupTextureParams() const noexcept;

        private:
            bool                isDepthBufferAttached   = false;
            bool                isStencilBufferAttached = false;
            GLuint              renderBuffer;
            std::vector<GLuint> textures;
            uint8_t             colorTextureCount       = 0;
            GLuint              width                   = 0xFFFFFFFF;
            GLuint              height                  = 0xFFFFFFFF;
        };
    };

};

#endif