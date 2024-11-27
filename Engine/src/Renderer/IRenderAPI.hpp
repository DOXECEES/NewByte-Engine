#ifndef SRC_RENDERER_IRENDERAPI_HPP
#define SRC_RENDERER_IRENDERAPI_HPP

#include <Windows.h>

#include "Camera.hpp"

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

        protected:
            HWND hwnd = {};
            Camera *cam;
        };
    };
};

#endif