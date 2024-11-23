#ifndef SRC_RENDERER_OPENGL_OPENGLRENDER_HPP
#define SRC_RENDERER_OPENGL_OPENGLRENDER_HPP

#include <Windows.h>

#include <glad/glad.h>
#include <GL/GL.h>
#include <gl/GLU.h>


#include "../RendererStructures.hpp"
#include "../Mesh.hpp"

#include "../IRenderAPI.hpp"
#include "../../Math/Matrix/Transformation.hpp"

#include "../../Debug.hpp"

#include "../../Manager/ResourceManager.hpp"

#include "../Camera.hpp"

#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126
#define WGL_DRAW_TO_WINDOW_ARB            0x2001
#define WGL_SUPPORT_OPENGL_ARB            0x2010
#define WGL_DOUBLE_BUFFER_ARB             0x2011
#define WGL_PIXEL_TYPE_ARB                0x2013
#define WGL_TYPE_RGBA_ARB                 0x202B
#define WGL_COLOR_BITS_ARB                0x2014
#define WGL_DEPTH_BITS_ARB                0x2022
#define WGL_SAMPLE_BUFFERS_ARB            0x2041
#define WGL_SAMPLES_ARB                   0x2042

namespace nb
{
    namespace OpenGl
    {
        class OpenGLRender : public nb::Renderer::IRenderAPI
        {

            typedef HGLRC (WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);
            typedef BOOL (WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

            private:

                OpenGLRender() = delete;
                OpenGLRender(HWND _hwnd) noexcept;
                ~OpenGLRender() noexcept;

                bool init() noexcept override;
                void initFail(std::string_view message, HGLRC context) noexcept;

            public:
                Ref<Renderer::Mesh> m = nullptr;

                void render() override;

                static OpenGLRender *create(HWND hwnd);

                static void setAmbientLight(const Math::Vector3<uint8_t>& color) noexcept
                {
                    ambientColor = Math::toFloatColor(color);
                }

                static void setLightPos(const Math::Vector3<float>& pos) noexcept
                {
                    lightPos = pos;
                }

            private:
                HDC hdc = {};
                static Math::Vector3<float> ambientColor;
                static Math::Vector3<float> lightPos;
        };
    };
};


#endif
