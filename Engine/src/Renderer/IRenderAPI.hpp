#ifndef SRC_RENDERER_IRENDERAPI_HPP
#define SRC_RENDERER_IRENDERAPI_HPP

#include <Windows.h>

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

            virtual void render() = 0;

        protected:
            HWND hwnd = {};
        };
    };
};

#endif