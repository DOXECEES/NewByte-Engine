#ifndef SRC_RENDERER_RENDERER_HPP
#define SRC_RENDERER_RENDERER_HPP

#include "../Core.hpp"

#include "IRenderAPI.hpp"
#include "OpenGL/OpenGLRender.hpp"

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

            inline void render() noexcept { api->render(); };

            ~Renderer() = default;

            void setCamera(Camera *cam) { api->setCamera(cam); };

        private:
            IRenderAPI * api;
            //Camera* camera;
        };
    };
};

#endif

