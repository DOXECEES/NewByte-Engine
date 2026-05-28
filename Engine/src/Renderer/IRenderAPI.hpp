#ifndef SRC_RENDERER_IRENDERAPI_HPP
#define SRC_RENDERER_IRENDERAPI_HPP

#include <NbCore.hpp>
#include <Windows.h>

#include <vector>
#include <Color.hpp>
#include "Core.hpp"
#include "IFrameBuffer.hpp"
#include "ContextMeshCache.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"
#include "PipelineCache.hpp"
#include "Renderer/Cubemap.hpp"
#include "Renderer/Texture.hpp"
#include "IUniformBuffer.hpp"


#include "Resources/MaterialAsset.hpp"
#include "Math/Vector3.hpp"
#include <Vector.hpp>
#include <Span.hpp>

namespace nb
{
    namespace Renderer
    {
        struct SharedWindowContext
        {
            HWND handle;
            HDC hdc;
            HGLRC hglrc;
        };

        enum class PolygonMode
        {
            POINTS,
            LINES,
            FULL
        };

        struct Pipeline
        {
            Ref<Shader> shader;
            PolygonMode polygonMode         = PolygonMode::FULL;
            bool        isDepthTestEnable   = true;
            bool        isBlendEnable       = true;
            bool        isCullingEnable     = false;
            
            bool        cullFront = false;
        };

        struct RendererCommand
        {
            Mesh*           mesh;
            std::vector<Ref<Resource::MaterialAsset>> material;
            PipelineHandle  pipeline;
            Math::Mat4<float>                         model;
            uint32 vertexCount = 0;
        };

        //struct BillboardComponent
        //{
        //    Math::Vector2<float>        size;
        //    Ref<Resource::TextureAsset> texture;
        //};

        struct BillboardCommand
        {
            Mesh*                mesh;
            PipelineHandle pipeline;

            Math::Vector3<float> pos;
            Ref<Resource::TextureAsset> texture;
        };

        struct Viewport
        {
            float x         = 0.0f;
            float y         = 0.0f;
            float width     = 0.0f;
            float height    = 0.0f;
            float minDepth  = 0.0f;
            float maxDepth  = 1.0f;
        };

        enum class TextureFormat
        {
            RGB,
            RGBA,
            RGB16F,            
        };
        struct TextureDescriptor
        {
            int width;
            int height;
            TextureFormat format;
            void* data;
        };

        enum class PrimitiveType
        {
            TRIANGLE,
            LINE,
        };

        struct VertexAttribute
        {
            uint32_t location; 
            uint32_t count;    
            uint32_t type;     
            uint32_t offset;   
        };

        struct VertexLayout
        {
            uint32_t                     stride; 
            std::vector<VertexAttribute> attributes;
        };

        class IRenderAPI
        {
        public:
            
            IRenderAPI(void* handle) noexcept;

            // NEW API

            virtual bool init(void* handle) noexcept = 0;
            virtual void beginFrame() noexcept = 0;
            virtual void endFrame() noexcept = 0;

            virtual void drawIndexedBuffer(
                nbstl::Span<const uint8_t> buffer,
                nbstl::Span<uint32_t>      indexBuffer,
                PrimitiveType    type,
                const VertexLayout&        layout
            ) noexcept                                                     = 0;
            virtual void drawMesh(const RendererCommand& command) noexcept = 0;
            virtual void drawVertexless(RendererCommand& command) noexcept = 0;

            virtual void drawContextMesh(const ContextMesh& contextMesh, PipelineHandle pipeline) noexcept = 0;

            virtual void bindPipeline(PipelineHandle pipelineHandle) noexcept = 0;

            virtual Ref<IFrameBuffer> createFrameBuffer(const uint32 width, const uint32 height) const noexcept = 0;

            virtual void bindDefaultFrameBuffer() noexcept = 0;
            virtual void bindFrameBuffer(const Ref<IFrameBuffer>& frameBuffer) noexcept = 0;

            virtual void bindTexture(uint8 slot, uint32 textureId) noexcept = 0;
            
            virtual void bindCubemap(
                uint8  slot,
                uint32 cubemapId
            ) noexcept = 0;

            virtual Ref<Cubemap> createCubemap(
                const CubemapParameters& params
            ) noexcept                                                                         = 0;  
            virtual Ref<Texture> createTexture2d(const TextureDescriptor& descriptor) noexcept = 0; 
            virtual Ref<Renderer::Cubemap> bakeTextureIntoCubeMap(Ref<Texture> texture2d) noexcept = 0;
            virtual Ref<Renderer::Cubemap> bakeIrradiance(Ref<Renderer::Cubemap> enviromentCubemap) noexcept = 0;
            virtual Ref<Renderer::Cubemap> bakePrefilter(Ref<Renderer::Cubemap> envCubemap) noexcept = 0;
            virtual Ref<Renderer::Texture> bakeBRDF() noexcept = 0;

            virtual Ref<nb::Renderer::Cubemap> bakePointLightMap(
                const nbstl::Vector<RendererCommand>& queue,
                Math::Vector3<float>                            lightPos,
                float                                           farPlane
            ) noexcept = 0;

           

            virtual void setViewport(const Viewport& viewport) noexcept = 0;
            virtual void clear(bool color, bool depth, bool stencil) noexcept = 0;
            
            virtual void setClearColor(const Color& color, float depthValue, int32 stencilValue) noexcept = 0;
           
            PipelineCache& getCache() noexcept;

            template<typename T> 
            const T& getNativeHandleAs() noexcept
            {
                if (!handle)
                {
                    nb::Error::ErrorManager::instance()
                        .report(nb::Error::Type::FATAL, "Handle does not link to RenderApi");
                    NB_ASSERT(handle, "Handle is nullptr");
                }
                return static_cast<T>(handle);
            }

            virtual SharedWindowContext shareContext(void* handle) const noexcept = 0;
            virtual void releaseContext(const SharedWindowContext& context) noexcept = 0;
            virtual bool setContext(HDC hdc, HGLRC hglrc) noexcept = 0;
            virtual bool setDefaultContext() noexcept = 0;
        public:
            inline Camera *getCamera() const noexcept { return cam; };
            void setCamera(Camera *c) { cam = c; };
            //virtual void render() = 0;

            virtual void setpolygonModePoints() noexcept  = 0;
            virtual void setPolygonModeLines()  noexcept  = 0;
            virtual void setPolygonModeFull()   noexcept  = 0;

            void enableLightVisualization() noexcept;
            void disableLightVisualization() noexcept;
            void toggleLightVisualization() noexcept;

        protected:
           

            void*                                   handle;
            Camera*                                 cam             = nullptr;

            PipelineHandle                          activePipeline  = 0;
            PipelineCache                           pipelineCache;

            bool shouldVisualizeLight = false;
        };
    };
};

#endif