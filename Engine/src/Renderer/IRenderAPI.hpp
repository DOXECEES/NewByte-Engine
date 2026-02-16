#ifndef SRC_RENDERER_IRENDERAPI_HPP
#define SRC_RENDERER_IRENDERAPI_HPP

#include <NbCore.hpp>
#include <Windows.h>

#include <vector>
#include "Color.hpp"
#include "IFrameBuffer.hpp"
#include "ContextMeshCache.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"
#include "SceneGraph.hpp"
#include "PipelineCache.hpp"

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

        };

        struct RendererCommand
        {
            Mesh*           mesh;
            PipelineHandle  pipeline;
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

        class IRenderAPI
        {
        public:
            
            IRenderAPI(void* handle) noexcept;

            // NEW API

            virtual bool init(void* handle) noexcept = 0;
            virtual void beginFrame() noexcept = 0;
            virtual void endFrame() noexcept = 0;

            virtual void drawMesh(RendererCommand& command) noexcept = 0;
            virtual void drawContextMesh(const ContextMesh& contextMesh, PipelineHandle pipeline) noexcept = 0;

            virtual void bindPipeline(PipelineHandle pipelineHandle) noexcept = 0;

            virtual Ref<IFrameBuffer> createFrameBuffer(const uint32 width, const uint32 height) const noexcept = 0;

            virtual void bindDefaultFrameBuffer() noexcept = 0;
            virtual void bindFrameBuffer(const Ref<IFrameBuffer>& frameBuffer) noexcept = 0;

            virtual void bindTexture(uint8 slot, uint32 textureId) noexcept = 0;


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
            virtual bool setContext(HDC hdc, HGLRC hglrc) noexcept = 0;
            virtual bool setDefaultContext() noexcept = 0;
        public:
            inline Camera *getCamera() const noexcept { return cam; };
            void setCamera(Camera *c) { cam = c; };
            //virtual void render() = 0;
            inline std::shared_ptr<SceneGraph> getScene() const noexcept { return sceneGraph; };
            inline void setScene(const std::shared_ptr<SceneGraph>&s) { sceneGraph = s; };

            virtual void setpolygonModePoints() noexcept  = 0;
            virtual void setPolygonModeLines()  noexcept  = 0;
            virtual void setPolygonModeFull()   noexcept  = 0;

            void enableLightVisualization() noexcept;
            void disableLightVisualization() noexcept;
            void toggleLightVisualization() noexcept;

        protected:
           

            void*                                   handle;
            Camera*                                 cam;

            PipelineHandle                          activePipeline  = 0;
            PipelineCache                           pipelineCache;
            std::shared_ptr<Renderer::SceneGraph>   sceneGraph;

            bool shouldVisualizeLight = false;
        };
    };
};

#endif