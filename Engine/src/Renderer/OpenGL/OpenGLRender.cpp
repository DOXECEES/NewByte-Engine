// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include <Windows.h>
#include "OpenGLRender.hpp"

#include "../Objects/Objects.hpp"
#include "DepthBuffer.hpp"


namespace nb::OpenGl
{

    GLenum OpenGLRender::toOpenGlPolygonMode(Renderer::PolygonMode mode) noexcept
    {
        switch (mode)
        {
        case nb::Renderer::PolygonMode::POINTS:
            return GL_POINTS;
        case nb::Renderer::PolygonMode::LINES:
            return GL_LINE;
        case nb::Renderer::PolygonMode::FULL:
            return GL_FILL;
        default:
            Error::ErrorManager::instance()
                .report(Error::Type::WARNING, "Unsupported polygon mode");
            return GL_FILL;
        }
    }

    Renderer::SharedWindowContext OpenGLRender::shareContext(void* handle) const noexcept
    {
        Renderer::SharedWindowContext ctx;
        ctx.handle = reinterpret_cast<HWND>(handle);
        ctx.hdc = GetDC(ctx.handle);

        if (!ctx.hdc) return {};

        int currentPF = GetPixelFormat(ctx.hdc);
        if (currentPF == 0)
        {
            if (!SetPixelFormat(ctx.hdc, this->pixelFormat, &this->pfd))
            {
                int fallbackPF = ChoosePixelFormat(ctx.hdc, &this->pfd);
                if (!SetPixelFormat(ctx.hdc, fallbackPF, &this->pfd))
                {
                    nb::Error::ErrorManager::instance()
                        .report(Error::Type::FATAL, "SetPixelFormat failed even with fallback")
                        .with("WinError", (int)GetLastError());
                    ReleaseDC(ctx.handle, ctx.hdc);
                    return {};
                }
            }
        }

        auto wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

        int contextAttribs[] = {
            0x2091, 4,      // WGL_CONTEXT_MAJOR_VERSION_ARB
            0x2092, 5,      // WGL_CONTEXT_MINOR_VERSION_ARB
            0x9126, 0x0001, // WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB
            0
        };

        ctx.hglrc = wglCreateContextAttribsARB(ctx.hdc, this->hglrc, contextAttribs);

        if (!ctx.hglrc)
        {
            ctx.hglrc = wglCreateContext(ctx.hdc);
            wglShareLists(this->hglrc, ctx.hglrc);
        }

        return ctx;
    }


    bool OpenGLRender::setContext(HDC hdc, HGLRC hglrc) noexcept
    {
        if (hdc == nullptr || hglrc == nullptr)
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::INFO, "HDC or HGLRC == nullptr");
            return wglMakeCurrent(this->hdc, this->hglrc);
        }
        return wglMakeCurrent(hdc, hglrc);
    }

    bool OpenGLRender::setDefaultContext() noexcept
    {
        return wglMakeCurrent(this->hdc, this->hglrc);
    }


    void OpenGLRender::beginFrame() noexcept
    {
        setClearColor(Renderer::Colors::WHITE, 1.0f, 0);
        clear(true, true, false);
    }

    void OpenGLRender::endFrame() noexcept
    {
        SwapBuffers(hdc);
    }

    void OpenGLRender::drawMesh(Renderer::RendererCommand& command) noexcept
    {
        bindPipeline(command.pipeline);
        command.mesh->draw(GL_TRIANGLES, pipelineCache.getDesc(activePipeline).shader);
    }

    void OpenGLRender::drawContextMesh(const Renderer::ContextMesh& contextMesh, Renderer::PipelineHandle pipeline) noexcept
    {
        bindPipeline(pipeline);
        contextMesh.source->draw(GL_TRIANGLES, pipelineCache.getDesc(activePipeline).shader, contextMesh.vao);
    }

    void OpenGLRender::bindPipeline(Renderer::PipelineHandle pipelineHandle) noexcept 
    {
        if (activePipeline == pipelineHandle)
        {
            return;
        }

        const auto& pipeline = pipelineCache.getDesc(pipelineHandle);

        pipeline.shader->use();

       
        glPolygonMode(GL_FRONT_AND_BACK, toOpenGlPolygonMode(pipeline.polygonMode));

        if (pipeline.isDepthTestEnable)
        {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }

        if (pipeline.isBlendEnable)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        else
        {
            glDisable(GL_BLEND);
        }

        activePipeline = pipelineHandle;
    }

    Ref<Renderer::IFrameBuffer> OpenGLRender::createFrameBuffer(const uint32 width, const uint32 height) const noexcept
    {
        auto buffer = createRef<FBO>();
        buffer->setSize(width, height);
        return buffer;
    }

    void OpenGLRender::bindDefaultFrameBuffer() noexcept
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0); 
    }

    void OpenGLRender::bindFrameBuffer(const Ref<Renderer::IFrameBuffer>& frameBuffer) noexcept
    {
        frameBuffer->bind();
    }

    void OpenGLRender::bindTexture(uint8 slot, uint32 textureId) noexcept
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, textureId);
    }

    void OpenGLRender::setViewport(const Renderer::Viewport& viewport) noexcept
    {
        glViewport(
            static_cast<GLuint>(viewport.x),
            static_cast<GLuint>(viewport.y),
            static_cast<GLuint>(viewport.width),
            static_cast<GLuint>(viewport.height)
        );
    }

    void OpenGLRender::clear(bool color, bool depth, bool stencil) noexcept
    {
        int32 clearFlag = 0;
        if (color)
        {
            clearFlag |= GL_COLOR_BUFFER_BIT;
        }
        if (depth)
        {
            clearFlag |= GL_DEPTH_BUFFER_BIT;
        }
        if (stencil)
        {
            clearFlag |= GL_STENCIL_BUFFER_BIT;
        }

        glClear(clearFlag);
    }

    void OpenGLRender::setClearColor(const Renderer::Color& color, float depthValue,  int32 stencilValue) noexcept
    {
        const Math::Vector4<GLfloat>& clearColor = color.asVec4();
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClearDepthf(depthValue);
        glClearStencil(stencilValue);
    }

};


nb::OpenGl::OpenGLRender::OpenGLRender(void* handle) noexcept
    : IRenderAPI(handle)
    , hdc(GetDC(static_cast<HWND>(handle)))
{
}

nb::OpenGl::OpenGLRender::~OpenGLRender() noexcept
{
    delete t;
    delete tn;
    wglMakeCurrent(nullptr, nullptr);
    ReleaseDC(getNativeHandleAs<HWND>(), hdc);
}



void GLAPIENTRY
MessageCallback(GLenum source,
                GLenum type,
                GLuint id,
                GLenum severity,
                GLsizei length,
                const GLchar *message,
                const void *userParam)
{
    //Debug::debug(message);
    nb::Error::ErrorManager::instance().report(nb::Error::Type::WARNING, message);
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}


bool nb::OpenGl::OpenGLRender::init(void* handle) noexcept
{
    this->handle = handle;
    // thats looks wierd, but works)
    WNDCLASS wc = {};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"DummyWGLWindow";

    RegisterClass(&wc);

    HWND dummyWindow = CreateWindow(
        L"DummyWGLWindow", L"Dummy", WS_OVERLAPPEDWINDOW,
        0, 0, 1, 1, nullptr, nullptr, wc.hInstance, nullptr);

    HDC dummyHDC = GetDC(dummyWindow);

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;

    int pixelFormat = ChoosePixelFormat(dummyHDC, &pfd);
    SetPixelFormat(dummyHDC, pixelFormat, &pfd);

    HGLRC tempContext = wglCreateContext(dummyHDC);
    wglMakeCurrent(dummyHDC, tempContext);

    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

    UINT numFormats;
    float fAttributes[] = {0, 0};

    int attributes[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
        WGL_SAMPLES_ARB, 4,
        0};

    int pixelFormatN = 0;

    if (!wglChoosePixelFormatARB(hdc, attributes, fAttributes, 1, &pixelFormatN, &numFormats) || numFormats == 0)
    {
        ReleaseDC(dummyWindow, dummyHDC);
        initFail("Failed to choose a multisample pixel format", nullptr);
        return false;
    }

    PIXELFORMATDESCRIPTOR pfdN;
    DescribePixelFormat(hdc, pixelFormatN, sizeof(pfdN), &pfdN);
    pixelFormat = pixelFormatN;
    pfd = pfdN;
    if (!SetPixelFormat(hdc, pixelFormatN, &pfdN))
    {
        initFail("Failed to set the pixel format", nullptr);
        return false;
    }

    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    if (!wglCreateContextAttribsARB)
    {
        initFail("Failed to get wglCreateContextAttribsARB", tempContext);
        return false;
    }

    int contextAttribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 5,
        WGL_CONTEXT_PROFILE_MASK_ARB, 0x0001,
        0};

    hglrc = wglCreateContextAttribsARB(hdc, nullptr, contextAttribs);

    if (!hglrc)
    {
        initFail("Failed to create modern OpenGL context", tempContext);
        return false;
    }

    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(tempContext);

    if (!wglMakeCurrent(hdc, hglrc))
    {
        initFail("Failed to set the modern OpenGL context", hglrc);
        return false;
    }

    if (!gladLoadGL())
    {
        initFail("Failed to initialize GLAD", hglrc);
        return false;
    }

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_CLAMP);
    //glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    ReleaseDC(dummyWindow, dummyHDC);
    DestroyWindow(dummyWindow);
    UnregisterClass(L"DummyWGLWindow", GetModuleHandle(nullptr));

    //loadScene();
    return true;
}

void nb::OpenGl::OpenGLRender::initFail(std::string_view message, HGLRC context) noexcept
{
    Debug::debug(message);
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(context);
    ReleaseDC(getNativeHandleAs<HWND>(), hdc);
}

void nb::OpenGl::OpenGLRender::loadScene() noexcept
{
    auto rm = nb::ResMan::ResourceManager::getInstance();
    auto shader = rm->getResource<nb::Renderer::Shader>("ADS.shader");

    //auto mesh = rm->getResource<Renderer::Mesh>("untitled2_.obj");
    //auto gizmo = rm->getResource<Renderer::Mesh>("Gizmo.obj");
    //auto lumine = rm->getResource<Renderer::Mesh>("Tactical_Lumine (2).obj");
    //auto wall = rm->getResource<Renderer::Mesh>("wall.obj");

    sceneGraph = Renderer::SceneGraph::getInstance();

    auto scene = sceneGraph->getScene();
    //auto wtr = Math::translate(Math::Mat4<float>::identity(), {0.0f, 0.0f, 0.0f});

    //Renderer::Transform at;

    //auto n = std::make_shared<Renderer::ObjectNode>("Weapon 1", at, mesh, shader);
    //auto g = std::make_shared<Renderer::ObjectNode>("Gizmo", at, gizmo, shader);

    //Renderer::Transform t;
    //t.translate = {0.0f, -10.0f, 0.0f};

    //Renderer::Transform t2;
    //t2.translate = {0.0f, 3.0f, 0.0f};
    //t2.rotateX = Math::toRadians(90.0f);

    //auto n5 = std::make_shared<Renderer::ObjectNode>("Lumine", t2, lumine, shader);

    //Renderer::Transform wallTransform;
    //wallTransform.translate = {0.0f, 0.0f, -15.0f};
    //wallTransform.scale = {8.0f, 1.0f, 1.0f};
    //wallTransform.rotateX = Math::toRadians(90.0f);

    //auto nWall = std::make_shared<Renderer::ObjectNode>("Wall", wallTransform , wall, shader);


  
    //auto box = rm->getResource<Renderer::Mesh>("box.obj");


    //Renderer::Transform boxTransform;
    //boxTransform.translate = {0.0f, 0.0f, 0.0f};
    //boxTransform.scale = {4.0f, 4.0f, 4.0f};

    //auto nBox = std::make_shared<Renderer::ObjectNode>("Box", boxTransform , box, shader);

    Renderer::Transform dirLightTransform;
    auto dirLight = std::make_shared<Renderer::DirectionalLight>(
        ambientColor,
        Math::Vector3<float>{1.0f, 1.0f, 1.0f},
        Math::Vector3<float>{0.1f, 0.1f, 0.1f},
        Math::Vector3<float>{-1.00f, 10.0f, -10.0f}
    );
    auto dirLightNode = std::make_shared<Renderer::LightNode>("DirLight", dirLightTransform, dirLight);

    Renderer::Transform pointLightTransform;
    pointLightTransform.translate = {0.0f, 0.0f, 0.0f};
    auto pointLight = std::make_shared<Renderer::PointLight>(
        ambientColor,
        Math::Vector3<float>{1.0f, 1.0f, 1.0f},
        Math::Vector3<float>{0.1f, 0.1f, 0.1f}, 
        pointLightTransform.translate,
        1.0f, 0.0f, 0.0f , 1.0f
    );
    auto pointLightNode = std::make_shared<Renderer::LightNode>("PointLight", pointLightTransform, pointLight);

    //auto hand = rm->getResource<Renderer::Mesh>("Hands.obj");
    //Renderer::Transform handsTransform;
    //auto hands = std::make_shared<Renderer::ObjectNode>("hands", handsTransform , hand, shader);

    /*static Ref<Renderer::Mesh> sniper = rm->getResource<Renderer::Mesh>("Sniper_Rifle.obj"); 
    Renderer::Transform sniperTransform;

    auto sniperNode = std::make_shared<Renderer::ObjectNode>("sniper", sniperTransform, sniper, shader);
    scene->addChildren(sniperNode);*/

    Renderer::PrimitiveGenerators::ParametricSegments segments{ 32, 32 };
    Ref<Renderer::Mesh> cube = Renderer::PrimitiveGenerators::createTorus(segments, 16,8);
    Renderer::Transform cubeTransform;
    cubeTransform.scale = { 0.25f,0.25f,0.25f };

    auto cubeNode = std::make_shared<Renderer::ObjectNode>("cube", cubeTransform, cube, shader);
    scene->addChildren(cubeNode);



    ////scene->addChildren(n);
    ////scene->addChildren(g);
    //scene->addChildren(n5);
    //n5->addChildren(nBox);
    ////scene->addChildren(nWall);
    ////scene->addChildren(hands);
    ////scene->addChildren(nBox);
    scene->addChildren(dirLightNode);
    dirLightNode->addChildren(pointLightNode);
    

}



void nb::OpenGl::OpenGLRender::visualizeLight(std::shared_ptr<Renderer::LightNode> node) const noexcept
{
    auto rm = ResMan::ResourceManager::getInstance();
    auto mesh = rm->getResource<Renderer::Mesh>("sphere.obj");

    auto lightVisulizeShader = rm->getResource<nb::Renderer::Shader>("lightVisulize.shader");
    
    Renderer::ShaderUniforms& uniforms = mesh->uniforms;

    uniforms.floatUniforms["time"] = Utils::Timer::timeElapsedSinceInit();
    uniforms.vec3Uniforms["viewPos"] = cam->getPosition();
    uniforms.mat4Uniforms["view"] = cam->getLookAt();
    uniforms.mat4Uniforms["proj"] = cam->getProjection();
    uniforms.mat4Uniforms["model"] = Math::translate(Math::Matrix<float, 4, 4>::identity(), node->getPosition() - Math::Vector3<float>{0.0f, 1.0f, 1.0f});

    mesh->draw(GL_TRIANGLES, lightVisulizeShader);

}

void nb::OpenGl::OpenGLRender::visualizeAabb(
    const Math::AABB3D &aabb,
    const Math::Mat4<float>& mat
) const noexcept
{

    auto rm = ResMan::ResourceManager::getInstance();
    auto aabbShader = rm->getResource<Renderer::Shader>("aabb.shader");

    Math::AABB3D B = Math::AABB3D::recalculateAabb3dByModelMatrix(aabb, mat);

    std::vector<Renderer::Vertex> vertices = {
        {{B.minPoint.x, B.minPoint.y, B.minPoint.z}, {}},
        {{B.minPoint.x, B.minPoint.y, B.maxPoint.z}, {}},
        {{B.maxPoint.x, B.minPoint.y, B.maxPoint.z}, {}},
        {{B.maxPoint.x, B.minPoint.y, B.minPoint.z}, {}},
        {{B.minPoint.x, B.maxPoint.y, B.minPoint.z}, {}},
        {{B.minPoint.x, B.maxPoint.y, B.maxPoint.z}, {}},
        {{B.maxPoint.x, B.maxPoint.y, B.maxPoint.z}, {}},
        {{B.maxPoint.x, B.maxPoint.y, B.minPoint.z}, {}}
    };

    static std::vector<unsigned int> edges = {
        0, 4,
        1, 5,
        2, 6,
        3, 7,

        0, 1,
        1, 2,
        2, 3,
        3, 0,

        4, 5,
        5, 6,
        6, 7,
        7, 4
    };

    Renderer::Mesh m(vertices, edges);
    m.uniforms.mat4Uniforms["model"] = Math::Mat4<float>::identity();
    m.uniforms.mat4Uniforms["view"] = cam->getLookAt();
    m.uniforms.mat4Uniforms["projection"] = cam->getProjection();

    m.draw(GL_LINES, aabbShader);
}



nb::OpenGl::OpenGLRender *nb::OpenGl::OpenGLRender::create(HWND hwnd)
{
    OpenGLRender *render = new OpenGLRender(hwnd);
    if (!render->init(hwnd))
    {
        delete render;
        return nullptr;
    }

    return render;
}

void nb::OpenGl::OpenGLRender::drawTransformationElements(const Ref<Renderer::Mesh>& mesh) noexcept
{
    // Math::AABB3D aabb = mesh->getAABB();
    // Math::Vector3<float> center = aabb.center();

    // Renderer::Mesh m({center, // Bottom-left
    //                   center + Math::Vector3<float>{10.0f,0.0f,0.0f},  // Bottom-right
    //                   center + Math::Vector3<float>{0.0f,10.0f,0.0f},  // Top-left
    //                   center + Math::Vector3<float>{0.0f,0.0f,10.0f}},

    //                  {0, 1,
    //                   0, 2,
    //                   0, 3 });

    auto rm = ResMan::ResourceManager::getInstance();
    auto arrowShader = rm->getResource<nb::Renderer::Shader>("arrow.shader");

    arrowShader->use();
    // m.draw(GL_LINES);
}

void nb::OpenGl::OpenGLRender::applyDefaultModel() noexcept
{
    //auto rm = nb::ResMan::ResourceManager::getInstance();
    //shader = rm->getResource<nb::Renderer::Shader>("ADS.shader");
}

void nb::OpenGl::OpenGLRender::applyDefaultModelFlat() noexcept
{
    //auto rm = nb::ResMan::ResourceManager::getInstance();
    //shader = rm->getResource<nb::Renderer::Shader>("vertFlat.shader");
}

void nb::OpenGl::OpenGLRender::applyDiffuseReflectionModel() noexcept
{
    //auto rm = nb::ResMan::ResourceManager::getInstance();
    //shader = rm->getResource<nb::Renderer::Shader>("DiffuseReflection.shader");
    //
    //shader->setUniformVec3("lightPos", {1.0f, 1.0f, 1.0f});
    //shader->setUniformVec3("Kd", ambientColor);
    //shader->setUniformVec3("Ld", {0.498f, 0.294f, 0.784f});
}

void nb::OpenGl::OpenGLRender::applyAmbientDiffuseSpecularModel(Renderer::Camera *cam) noexcept
{
    //auto rm = nb::ResMan::ResourceManager::getInstance();
    //shader = rm->getResource<nb::Renderer::Shader>("ADS.shader");
    //shader->setUniformVec3("La", {1.f, 1.f, 1.f});
    //shader->setUniformVec3("Ka", mat.ambient);

    //shader->setUniformVec3("Ld", {1.f, 1.f, 1.f});
    //shader->setUniformVec3("Kd", mat.diffuse);
    //shader->setUniformVec3("LightPos", {1.2f, 1.0f, 2.0f});

    //// shader->setUniformMat4("ViewDir", cam->getLookAt() );
    //shader->setUniformVec3("Ls", {1.0f, 1.0f, 1.0f});
    //shader->setUniformVec3("Ks", mat.specular);
    //shader->setUniformFloat("shine", mat.shininess);

    //shader->setUniformVec3("viewPos", cam->getPosition());
}

void nb::OpenGl::OpenGLRender::setpolygonModePoints() noexcept
{
    polygonMode = GL_POINT;
}

void nb::OpenGl::OpenGLRender::setPolygonModeLines() noexcept
{
    polygonMode = GL_LINE;
}

void nb::OpenGl::OpenGLRender::setPolygonModeFull() noexcept
{
    polygonMode = GL_FILL;    
}

void nb::OpenGl::OpenGLRender::refreshPolygonMode() const noexcept
{
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
}



nb::Math::Vector3<float> nb::OpenGl::OpenGLRender::ambientColor = {0.0f, 0.0f, 0.0f};
nb::Math::Mat4<float> nb::OpenGl::OpenGLRender::model({{1.0f, 0.f, 0.f, 0.f},
                                                       {0.f, 1.0f, 0.f, 0.f},
                                                       {0.f, 0.f, 1.0f, 0.f},
                                                       {0.f, 0.f, 0.f, 1.0f}});

nb::Renderer::Material nb::OpenGl::OpenGLRender::mat;

//
nb::Math::Mat4<float> nb::OpenGl::OpenGLRender::gizmoModelMat;
