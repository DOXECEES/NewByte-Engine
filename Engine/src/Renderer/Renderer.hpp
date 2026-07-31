#ifndef SRC_RENDERER_RENDERER_HPP
#define SRC_RENDERER_RENDERER_HPP

#include "../Core.hpp"

#include <Vector.hpp>
#include <NonOwningPtr.hpp>

#include "../Math/Vector3.hpp"
#include "../Math/Vector2.hpp"


#include "IRenderAPI.hpp"
#include "Math/Vector4.hpp"
#include "OpenGL/OpenGLRender.hpp"
#include "Renderer/ContextMeshCache.hpp"

#include "Mesh.hpp"

#include "Camera.hpp"
#include "Resources/MaterialAsset.hpp"
#include "ECS/Ecs.hpp"

#include <tiny-gizmo.hpp>
#include "Cubemap.hpp"

#include "GBuffer.hpp"
#include "SSAO.hpp"

//
#include "OpenGL/Placeholder.hpp"
#include "Scene.hpp"
#include "Light.hpp"

#include "IUniformBuffer.hpp"
#include "JoltDebugRenderer.hpp"
#include "PostProcess.hpp"
//

namespace nb::Physics
{
    class PhysicsSystem;
}

namespace nb
{
    namespace Renderer
    {
        class JoltDebugRenderer;

        struct DebugRendererSettings 
        {
            bool showCameraFrustrum = true;
            bool showDebugBillboards = true;
            bool showLightGizmos = true;
            bool showGizmo = true;
            bool showColliders = false;
        };

        class Renderer 
        {
        public:
            constexpr static uint32_t COUNT_OF_POST_EFFECTS = 4;

            Renderer() = delete;
            Renderer(HWND hwnd, nb::Core::GraphicsAPI apiType) noexcept;

            void onResize(uint32 width, uint32 heigth) noexcept;
            void render(nbstl::NonOwningPtr<Camera> camera) noexcept;

            ~Renderer() = default;

            void togglePolygonVisibilityMode(PolygonMode mode) const noexcept;
            inline void setPolygonVisibilityMode(PolygonMode mode) noexcept     { polygonMode = mode; };

            void pickNodeAndApplyMaterial(int x, int y, const std::filesystem::path& path) noexcept;

            void setWireframeMode(bool flag) noexcept;
            void showVertexColor(bool flag) noexcept;

            void setDebugSettings(const DebugRendererSettings& settings) noexcept;

            inline void toggleSsao() noexcept
            {
                useSsao = !useSsao;
            }

            inline void toggleDebugPass() noexcept { isDebugPassEnabled = !isDebugPassEnabled; }
            inline void toggleBoundingBoxVisualization() noexcept
            {
                isBoundingBoxVisualizationEnabled = !isBoundingBoxVisualizationEnabled;
            }

            inline void toggleGridShow() noexcept
            {
                isShowGridEnabled = !isShowGridEnabled;
            }
            inline void toggleBvhVisualization() noexcept
            {
                isBVHVisualizationEnabled = !isBVHVisualizationEnabled;
            }

            bool isResourceReady() const noexcept;
        
            SharedWindowContext createSharedContextForWindow(HWND handle) noexcept;
            void releaseSharedContextForWindow(const SharedWindowContext& context) noexcept;

            struct TexturePreviewRequest
            {
                uint64 source;
                nb::Math::Vector3<float> channelMask = {1.0f, 1.0f, 1.0f};
                float gamma = 2.2f;
                float exposure = 1.0f;
            };

            void renderFramebufferToContext(
                const SharedWindowContext& out,
                const Ref<IFrameBuffer>&   framebuffer,
                uint32_t                   attachmentIndex = 0
            ) noexcept;


            void blitToWindow(const SharedWindowContext& out, const TexturePreviewRequest& request);

             void renderShadowPreview(
                const SharedWindowContext& out,
                uint64_t                   shadowTextureId,
                float                      nearPlane,
                float                      farPlane
            );

            struct MaterialPreviewRequest
            {
                Resource::MaterialAsset* material;
                float x = 0.0f;
                float y = 0.0f;
            };
            
            
            void renderMaterialPreview(const SharedWindowContext& out, MaterialPreviewRequest& request);

            struct ThumbnailImageData
            {
                std::vector<unsigned char> data;
                int                        width;
                int                        height;
            };



            void saveSpherePreview(
                const std::filesystem::path& materialPath,
                const std::string&           savePath
            );
            // TEMP

            void outline(Node node) noexcept;
            

            

            uint32 getGizmoTextureId() const
            {
                return navigationalGizmoFrameBuffer->getTexture();
            }

            uint32 getShadowTextureId() const
            {
                return shadowFrameBuffer->getTexture();
            }

            IRenderAPI* getApi() const noexcept
            {
                return api;
            }

            void setCheckedTextureId(uint32_t id) { checkedTextureId = id; }

            tinygizmo::gizmo_context& getGizmoContext() noexcept;

            uint64_t ssaoResult = 0;

            bool useSsao = true;

            const SSAOConfig& getSSAOConfig() const noexcept
            {
                return ssao->getConfig();
            }

            SSAOConfig& getSSAOConfig() noexcept
            {
                return ssao->getConfig();
            }

            PostProcess& getPostProcess() noexcept { return *postProcess; }

            struct PostProcessConfig
            {
                bool isLutEnabled = true;
                bool isSSREnabled = false;
                bool isDepthOfFieldEnabled = false;
            };

            const PostProcessConfig& getPostProcessConfig() const noexcept
            {
                return postProcessConfig;
            }

            PostProcessConfig& getPostProcessConfig() noexcept
            {
                return postProcessConfig;
            }

            Ref<Mesh> drawLine(
                const Math::Vector3<float>& p1,
                const Math::Vector3<float>& p2
            ) noexcept;


            static void generatePreviewForMaterial(const std::filesystem::path& path) noexcept
            {
                previewQueue.pushBack(path);
            }

            void setPhysicsSystem(nb::Physics::PhysicsSystem* physicsSystem) noexcept
            {
                mPhysicsSystem = physicsSystem;
            }

            void drawJoltGeometry(const Ref<Mesh>& mesh, const nb::Math::Mat4<float>& modelMatrix, bool isWireframe) noexcept;



        private:


            void renderDebugPasses(
                const nb::Math::Mat4<float>&          view,
                const nb::Math::Mat4<float>&          proj,
                const std::vector<Ecs::EntityID>&     dirLights,
                const std::vector<Ecs::EntityID>&     pointLights,
                const nbstl::Vector<RendererCommand>& mainQueue
            ) noexcept;

            void renderSSR(
                int                          width,
                int                          height,
                const nb::Math::Mat4<float>& view,
                const nb::Math::Mat4<float>& proj
            ) noexcept;

            void renderFinalQuad(
                int width,
                int height
            ) noexcept;

            
            

        private:

            nb::Node activeNode = Node();

            tinygizmo::gizmo_context gizmoCtx;

            void renderNavigationalGizmo() noexcept;

            //TEMP
            void loadSceneEcs() noexcept;
          
            uint32_t checkedTextureId = 0;

            std::unique_ptr<GBuffer> gBuffer = nullptr;

            Ref<Mesh> debugLightMesh = nullptr;
            Ref<Shader> debugLightShader = nullptr;

            bool isDebugPassEnabled = false;
            bool isBoundingBoxVisualizationEnabled = false;
            bool isShowGridEnabled = true;
            bool isBVHVisualizationEnabled = false;

            SharedWindowContext ctx;
            Ref<ContextMeshCache> contextMeshCache = nullptr;

            SSAO* ssao;
            PostProcessConfig postProcessConfig = {};
            //bool  isSSAOEnabled = true;

        private:

            DebugRendererSettings debugRendererSettings;
            std::unique_ptr<PostProcess> postProcess = nullptr;

            
            std::unique_ptr<Skybox> skybox;
            inline static nbstl::Vector<std::filesystem::path> previewQueue;

            bool    isResourceLoaded     = false;
            bool    isPreviewInitialized = false;
            nbstl::NonOwningPtr<Camera> cachedCamera;

            Ref<IFrameBuffer> mainFrameBuffer;         
            Ref<IFrameBuffer> ssrResultBuffer;
            Ref<IFrameBuffer> ssrBlurBuffer;

            Ref<IFrameBuffer> shadowFrameBuffer;
            Ref<IFrameBuffer> pointShadowFrameBuffer;

            Ref<IFrameBuffer> navigationalGizmoFrameBuffer;
            Ref<IFrameBuffer> outlineMaskFrameBuffer;

            struct PointLightData
            {
                PointLightProxy pointLight[32];
                int countOfpointLight;
                float padding[3];
            };

            std::unique_ptr<IUniformBuffer<PointLightData>> pointLightUbo;

            nb::Physics::PhysicsSystem* mPhysicsSystem = nullptr; 
            std::unique_ptr<JoltDebugRenderer> mJoltDebugRenderer;

            PolygonMode polygonMode;
            IRenderAPI* api;

            Ref<Mesh> quadScreenMesh;


            std::unordered_map<Ecs::EntityID, Ref<nb::Renderer::Cubemap>> m_pointShadowMaps;

        };
    };
};


#endif

