#ifndef SRC_RENDERER_IRENDERAPI_HPP
#define SRC_RENDERER_IRENDERAPI_HPP

#include <Windows.h>
#include <vector>

#include "Camera.hpp"
#include "Mesh.hpp"
#include "SceneGraph.hpp"

namespace nb
{
    namespace Renderer
    {
        class IRenderAPI
        {
        protected:
            
            IRenderAPI() = delete;
            IRenderAPI(HWND _hwnd) noexcept;
            virtual bool init() noexcept = 0;


        public:
            inline Camera *getCamera() const noexcept { return cam; };
            void setCamera(Camera *c) { cam = c; };
            virtual void render() = 0;
            inline std::shared_ptr<Renderer::SceneGraph> getScene() const noexcept { return sceneGraph; };
            inline void setScene(const std::shared_ptr<Renderer::SceneGraph>&s) { sceneGraph = s; };

            virtual void setpolygonModePoints() noexcept  = 0;
            virtual void setPolygonModeLines()  noexcept  = 0;
            virtual void setPolygonModeFull()   noexcept  = 0;

        protected:
            HWND                                    hwnd        = {};
            Camera*                                 cam;

            std::shared_ptr<Renderer::SceneGraph>   sceneGraph;
        };
    };
};

#endif