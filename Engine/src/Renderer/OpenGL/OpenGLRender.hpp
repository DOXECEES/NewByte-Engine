#ifndef SRC_RENDERER_OPENGL_OPENGLRENDER_HPP
#define SRC_RENDERER_OPENGL_OPENGLRENDER_HPP

#include <Windows.h>

#include <glad/glad.h>
#include <GL/GL.h>
#include <gl/GLU.h>

#include "../IRenderAPI.hpp"

#include "../../Debug.hpp"

#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

namespace nb
{
    namespace OpenGl
    {
        class OpenGLRender : public nb::Renderer::IRenderAPI
        {

            typedef HGLRC (WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);
            
            private:

                OpenGLRender() = delete;
                OpenGLRender(HWND _hwnd) noexcept;
                ~OpenGLRender() noexcept;

                bool init() noexcept override;
                void initFail(std::string_view message, HGLRC context) noexcept;

            public:
                
                void render() override;

                static OpenGLRender *create(HWND hwnd);

            private:
                HDC hdc = {};
        };
    };
};


#endif