#ifndef SRC_RENDERER_RENDERER_HPP
#define SRC_RENDERER_RENDERER_HPP

#include "../Core.hpp"

#include "../Math/Vector3.hpp"
#include "../Math/Vector2.hpp"

#include "SceneGraph.hpp"

#include "IRenderAPI.hpp"
#include "OpenGL/OpenGLRender.hpp"

#include "Mesh.hpp"

#include "Camera.hpp"

namespace nb
{
    namespace Renderer
    {
        class Renderer 
        {
        public:
            enum class PolygonMode
            {
                POINTS,
                LINES,
                FULL
            };

            Renderer() = delete;
            Renderer(HWND hwnd, nb::Core::GraphicsAPI apiType) noexcept;

            inline void render() noexcept                                       { api->render(); };

            ~Renderer() = default;

            void setCamera(Camera *cam)                                         { api->setCamera(cam); };
            inline Camera *getCamera() const noexcept                           { return api->getCamera(); };
            inline const std::shared_ptr<SceneGraph> getScene() const noexcept  { return api->getScene(); };
            inline void setScenes(std::shared_ptr<SceneGraph> &s)               { api->setScene(s); };

            void togglePolygonVisibilityMode(PolygonMode mode) const noexcept;
            inline void setPolygonVisibilityMode(PolygonMode mode) noexcept     { polygonMode = mode; };

        private:
            PolygonMode polygonMode;
            IRenderAPI* api;
            //Camera* camera;
        };
    };
};

#endif

