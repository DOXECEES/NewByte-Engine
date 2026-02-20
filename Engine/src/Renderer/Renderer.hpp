#ifndef SRC_RENDERER_RENDERER_HPP
#define SRC_RENDERER_RENDERER_HPP

#include "../Core.hpp"

#include <Vector.hpp>

#include "../Math/Vector3.hpp"
#include "../Math/Vector2.hpp"

#include "SceneGraph.hpp"

#include "IRenderAPI.hpp"
#include "OpenGL/OpenGLRender.hpp"
#include "Renderer/ContextMeshCache.hpp"

#include "Mesh.hpp"

#include "Camera.hpp"

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
            inline const std::shared_ptr<SceneGraph>& getScene() const noexcept  { return sceneGraph; };
            inline void setScenes(std::shared_ptr<SceneGraph> &s)               { sceneGraph = s; };

            void togglePolygonVisibilityMode(PolygonMode mode) const noexcept;
            inline void setPolygonVisibilityMode(PolygonMode mode) noexcept     { polygonMode = mode; };

            void setWireframeMode(bool flag) noexcept;
            void showVertexColor(bool flag) noexcept;

            inline void toggleDebugPass() noexcept { isDebugPassEnabled = !isDebugPassEnabled; }

            bool isResourceReady() const noexcept;
        
            SharedWindowContext createSharedContextForWindow(HWND handle) noexcept;
            void blitToWindow(const SharedWindowContext& out, uint32 textureId);

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


            void setCheckedTextureId(uint32_t id) { checkedTextureId = id; }


        private:

            void renderNavigationalGizmo() noexcept;

            //TEMP
            void loadScene() noexcept;
          
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

            SharedWindowContext ctx;
            Ref<ContextMeshCache> contextMeshCache = nullptr;

        private:

            bool isResourceLoaded = false;

            Ref<SceneGraph> sceneGraph;
            Camera* cam;

            Ref<IFrameBuffer> mainFrameBuffer;         
            Ref<IFrameBuffer> shadowFrameBuffer;
            Ref<IFrameBuffer> navigationalGizmoFrameBuffer;

            PolygonMode polygonMode;
            IRenderAPI* api;
            //Camera* camera;

            Ref<Mesh> quadScreenMesh;
        };
    };
};

#endif

