#ifndef SRC_RENDERER_OPENGL_OPENGLRENDER_HPP
#define SRC_RENDERER_OPENGL_OPENGLRENDER_HPP

#include <Windows.h>

#include <glad/glad.h>
#include <GL/GL.h>
#include <gl/GLU.h>

#include "../../Math/Matrix/Transformation.hpp"
#include "../../Math/Math.hpp"

#include "../RendererStructures.hpp"
#include "../Mesh.hpp"


#include "../IRenderAPI.hpp"

#include "../../Debug.hpp"

#include "../../Manager/ResourceManager.hpp"

#include "../Camera.hpp"

#include "../Skybox.hpp"

#include "OpenGLTexture.hpp"
#include "ShaderConstants.hpp"

#include "Renderer/OpenGL/FBO.hpp"
#include "DepthBuffer.hpp"
#include "PostProcessFbo.hpp"

#include "../../Utils/Timer.hpp"

#include <memory>

#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126
#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_COLOR_BITS_ARB                      0x2014
#define WGL_DEPTH_BITS_ARB                      0x2022
#define WGL_SAMPLE_BUFFERS_ARB                  0x2041
#define WGL_SAMPLES_ARB                         0x2042


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
                OpenGLRender(void* handle) noexcept;
                ~OpenGLRender() noexcept;

                void beginFrame() noexcept override;
                void endFrame() noexcept override;

                void drawMesh(Renderer::RendererCommand& command) noexcept override;
                void drawContextMesh(const Renderer::ContextMesh& contextMesh, Renderer::PipelineHandle pipeline) noexcept override;


                void bindPipeline(Renderer::PipelineHandle pipelineHandle) noexcept override;

                Ref<Renderer::IFrameBuffer> createFrameBuffer(const uint32 width, const uint32 height) const noexcept override;
                void bindDefaultFrameBuffer() noexcept override;
                void bindFrameBuffer(const Ref<Renderer::IFrameBuffer>& frameBuffer) noexcept override;

                void bindTexture(uint8 slot, uint32 textureId) noexcept override;


                void setViewport(const Renderer::Viewport& viewport) noexcept override;

                void clear(bool color, bool depth, bool stencil) noexcept override;

                void setClearColor(const Renderer::Color& color, float depthValue, int32 stencilValue) noexcept override;

                bool init(void* handle) noexcept override;
                void initFail(std::string_view message, HGLRC context) noexcept;

                static GLenum toOpenGlPolygonMode(Renderer::PolygonMode mode) noexcept;

                Renderer::SharedWindowContext shareContext(void* handle) const noexcept override;
                bool setContext(HDC hdc, HGLRC hglrc) noexcept override;
                bool setDefaultContext() noexcept override;



                void visualizeAabb(
                    const Math::AABB3D& aabb,
                    const Math::Mat4<float>& mat
                ) const noexcept;

            public:

      
                static OpenGLRender *create(HWND hwnd);

                static void setAmbientLight(const Math::Vector3<uint8_t>& color) noexcept
                {
                    ambientColor = Math::toFloatColor(color);
                }

                static void drawTransformationElements(const Ref<Renderer::Mesh>& mesh) noexcept;

                static void applyDefaultModel() noexcept;
                static void applyDefaultModelFlat() noexcept;
                static void applyDiffuseReflectionModel() noexcept;
                static void applyAmbientDiffuseSpecularModel(Renderer::Camera* cam) noexcept;
                
                

                virtual void setpolygonModePoints() noexcept override;
                virtual void setPolygonModeLines()  noexcept override;
                virtual void setPolygonModeFull()   noexcept override;
                

                void refreshPolygonMode() const noexcept;
                


                static Math::Mat4<float>        gizmoModelMat;

                inline static int               countOfDraws    = 0;

                //

            private:

                PIXELFORMATDESCRIPTOR pfd = {};
                int pixelFormat = {};
                HGLRC hglrc = {};
                HDC hdc = {};
                
                static Math::Vector3<float>         ambientColor;
                static Math::Mat4<float>            model;

                static Renderer::Material           mat;

                ///
                int polygonMode = GL_FILL;
                ///

                // temp
                std::vector<Math::Vector3<float>>   lightPosition;
                bool                                shouldVisualizeAabb     = false;

                //

                OpenGlTexture*                      t                       = nullptr;
                OpenGlTexture*                      tn                      = nullptr;
                std::shared_ptr<PostProcessFbo>     postProcessFbo          = nullptr;

              
                

};
    };
};


#endif
