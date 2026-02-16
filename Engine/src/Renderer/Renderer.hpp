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

            void setCamera(Camera *cam)                                         { this->cam = cam; };
            inline Camera *getCamera() const noexcept                           { return cam; };
            inline const std::shared_ptr<SceneGraph> getScene() const noexcept  { return sceneGraph; };
            inline void setScenes(std::shared_ptr<SceneGraph> &s)               { sceneGraph = s; };

            void togglePolygonVisibilityMode(PolygonMode mode) const noexcept;
            inline void setPolygonVisibilityMode(PolygonMode mode) noexcept     { polygonMode = mode; };

            inline void toggleDebugPass() noexcept { isDebugPassEnabled = !isDebugPassEnabled; }

        
            SharedWindowContext createSharedContextForWindow(HWND handle) noexcept;
            void blitToWindow(const SharedWindowContext& out, uint32 textureId);

        private:

            //TEMP
            void loadScene() noexcept;
          

            OpenGl::OpenGlTexture* t = nullptr;
            OpenGl::OpenGlTexture* tn = nullptr;


            std::shared_ptr<OpenGl::OpenGlTexture> albedo;
            std::shared_ptr<OpenGl::OpenGlTexture> metal;
            std::shared_ptr<OpenGl::OpenGlTexture> roughtness;
            std::shared_ptr<OpenGl::OpenGlTexture> ao;
            std::shared_ptr<OpenGl::OpenGlTexture> normal;

            Ref<Mesh> debugLightMesh = nullptr;
            Ref<Shader> debugLightShader = nullptr;

            bool isDebugPassEnabled = true;

            SharedWindowContext ctx;
            Ref<ContextMeshCache> contextMeshCache = nullptr;

        private:

            Ref<SceneGraph> sceneGraph;
            Camera* cam;

            Ref<IFrameBuffer> mainFrameBuffer;         
            Ref<IFrameBuffer> shadowFrameBuffer;

            PolygonMode polygonMode;
            IRenderAPI* api;
            //Camera* camera;

            Ref<Mesh> quadScreenMesh;
        };
    };
};

#endif

