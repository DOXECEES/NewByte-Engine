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

#include "../SceneGraph.hpp"

#include "../IRenderAPI.hpp"
#include "../../Math/Matrix/Transformation.hpp"

#include "../../Debug.hpp"

#include "../../Manager/ResourceManager.hpp"

#include "../Camera.hpp"

#include "../Skybox.hpp"

#include "OpenGLTexture.hpp"
#include "ShaderConstants.hpp"

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
                OpenGLRender(HWND _hwnd) noexcept;
                ~OpenGLRender() noexcept;

                bool init() noexcept override;
                void initFail(std::string_view message, HGLRC context) noexcept;
                void loadScene() noexcept;

                void visualizeLight(std::shared_ptr<Renderer::LightNode> node) const noexcept;
                void visualizeAabb(const Math::AABB3D& aabb, Math::Mat4<float> mat) const noexcept;

            public:

                void render() override;
                void renderNode(std::shared_ptr<Renderer::BaseNode> node
                    , const std::vector<Renderer::LightNode>& lightNode
                    , const OpenGl::DepthBuffer& depthBuffer
                    , const Math::Mat4<float>& lightSpaceMatrix
                ) noexcept;

                static OpenGLRender *create(HWND hwnd);

                static void setAmbientLight(const Math::Vector3<uint8_t>& color) noexcept
                {
                    ambientColor = Math::toFloatColor(color);
                }

                static void drawTransformationElements(const Ref<Renderer::Mesh> mesh) noexcept;

                static void applyDefaultModel() noexcept;
                static void applyDefaultModelFlat() noexcept;
                static void applyDiffuseReflectionModel() noexcept;
                static void applyAmbientDiffuseSpecularModel(Renderer::Camera* cam) noexcept;
                
                // bad movement -> refactor graph or sthl this
                static void spawnGizmo(const Renderer::ObjectNode& node) noexcept
                {
                    Math::AABB3D aabb = node.mesh->getAabb3d();
                    gizmoModelMat = Math::translate(Math::Mat4<float>::identity(), node.getPosition() + aabb.center());
                }

                virtual void setpolygonModePoints() noexcept override;
                virtual void setPolygonModeLines()  noexcept override;
                virtual void setPolygonModeFull()   noexcept override;

                void refreshPolygonMode() const noexcept;
                

                static Math::Mat4<float>        MVP;
                static Ref<Renderer::Shader>    shader;

                static Math::Mat4<float>        gizmoModelMat;

                inline static int               countOfDraws    = 0;

                //

            private:

                HDC hdc = {};
                
                static Math::Vector3<float>         ambientColor;
                static Math::Vector3<float>         lightPos;
                static Math::Mat4<float>            model;

                static Renderer::Material           mat;

                ///
                int polygonMode = GL_FILL;
                ///

                // temp
                std::vector<Math::Vector3<float>>   lightPosition;
                bool                                shouldVisualizeLight    = false;
                bool                                shouldVisualizeAabb     = false;

                //

                OpenGlTexture*                      t                       = nullptr;
                OpenGlTexture*                      tn                      = nullptr;
                std::shared_ptr<PostProcessFbo>     postProcessFbo          = nullptr;

        };
    };
};


#endif
