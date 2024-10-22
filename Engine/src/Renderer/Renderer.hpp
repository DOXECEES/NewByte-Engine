#ifndef SRC_RENDERER_RENDERER_HPP
#define SRC_RENDERER_RENDERER_HPP

#include "../Core.hpp"

#include "IRenderAPI.hpp"
#include "OpenGL/OpenGLRender.hpp"

namespace nb
{
    namespace Renderer
    {
        class Renderer 
        {
        public:
            enum class GraphicsAPI
            {
                OPENGL,
                DIRECTX,
                VULKAN
            };

            Renderer() = delete;
            Renderer(HWND hwnd, GraphicsAPI apiType) noexcept;

            inline void render() noexcept { api->render(); };

            ~Renderer();



        private:
            IRenderAPI * api;
        };
    };
};

#endif

