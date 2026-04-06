#ifndef SRC_RENDERER_RENDERER_HPP
#define SRC_RENDERER_RENDERER_HPP

#include "../Core.hpp"

#include <Vector.hpp>

#include "../Math/Vector3.hpp"
#include "../Math/Vector2.hpp"


#include "IRenderAPI.hpp"
#include "Math/Vector4.hpp"
#include "OpenGL/OpenGLRender.hpp"
#include "Renderer/ContextMeshCache.hpp"

#include "Mesh.hpp"

#include "Camera.hpp"
#include "Resources/MaterialAsset.hpp"

#include <tiny-gizmo.hpp>

namespace nb
{
    namespace Renderer
    {
        class Renderer 
        {
        public:

            Renderer() = delete;
            Renderer(HWND hwnd, nb::Core::GraphicsAPI apiType) noexcept;

            void onResize(uint32 width, uint32 heigth) noexcept;
            void render() noexcept;

            ~Renderer() = default;

            void setCamera(Camera* camera)                                      { this->cam = camera; };
            inline Camera *getCamera() const noexcept                           { return cam; };

            void togglePolygonVisibilityMode(PolygonMode mode) const noexcept;
            inline void setPolygonVisibilityMode(PolygonMode mode) noexcept     { polygonMode = mode; };

            void pickNodeAndApplyMaterial(int x, int y, const std::filesystem::path& path) noexcept;

            void setWireframeMode(bool flag) noexcept;
            void showVertexColor(bool flag) noexcept;

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
                Texture* source = nullptr;
                nb::Math::Vector3<float> channelMask = {1.0f, 1.0f, 1.0f};
                float gamma = 2.2f;
                float exposure = 1.0f;
            };


            void blitToWindow(const SharedWindowContext& out, const TexturePreviewRequest& request);

            struct MaterialPreviewRequest
            {
                Resource::MaterialAsset* material;
                float x = 0.0f;
                float y = 0.0f;
            };
            void renderMaterialPreview(const SharedWindowContext& out, MaterialPreviewRequest& request);


            // TEMP

            uint32 getAlbedoId() const
            {
                return albedo->getId();
            }

            uint32 getMetalId() const
            {
                return metal->getId();
            }

            uint32 getRoughtnessId() const
            {
                return roughtness->getId();
            }

            uint32 getAoId() const
            {
                return ao->getId();
            }

            uint32 getNormalId() const
            {
                return normal->getId();
            }

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



        private:
            tinygizmo::gizmo_context gizmoCtx;

            void renderNavigationalGizmo() noexcept;

            //TEMP
            void loadSceneEcs() noexcept;
          
            uint32_t checkedTextureId = 0;

            OpenGl::OpenGlTexture* t = nullptr;
            OpenGl::OpenGlTexture* tn = nullptr;


            std::shared_ptr<OpenGl::OpenGlTexture> albedo;
            std::shared_ptr<OpenGl::OpenGlTexture> metal;
            std::shared_ptr<OpenGl::OpenGlTexture> roughtness;
            std::shared_ptr<OpenGl::OpenGlTexture> ao;
            std::shared_ptr<OpenGl::OpenGlTexture> normal;

           

            Ref<Mesh> debugLightMesh = nullptr;
            Ref<Shader> debugLightShader = nullptr;

            bool isDebugPassEnabled = false;
            bool isBoundingBoxVisualizationEnabled = false;
            bool isShowGridEnabled = true;
            bool isBVHVisualizationEnabled = false;

            SharedWindowContext ctx;
            Ref<ContextMeshCache> contextMeshCache = nullptr;

        private:


            bool isResourceLoaded = false;

            Camera* cam;

            Ref<IFrameBuffer> mainFrameBuffer;         
            Ref<IFrameBuffer> shadowFrameBuffer;
            Ref<IFrameBuffer> navigationalGizmoFrameBuffer;

            PolygonMode polygonMode;
            IRenderAPI* api;

            Ref<Mesh> quadScreenMesh;
        };
    };
};





#endif

