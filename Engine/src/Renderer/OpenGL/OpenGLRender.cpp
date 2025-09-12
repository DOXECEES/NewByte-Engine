// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "OpenGLRender.hpp"

#include "../Objects/Objects.hpp"
#include "DepthBuffer.hpp"

nb::OpenGl::OpenGLRender::OpenGLRender(HWND _hwnd) noexcept
    : IRenderAPI(_hwnd), hdc(GetDC(hwnd))
{
}

nb::OpenGl::OpenGLRender::~OpenGLRender() noexcept
{
    delete t;
    delete tn;
    delete postProcessFbo;
    wglMakeCurrent(nullptr, nullptr);
    ReleaseDC(hwnd, hdc);
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
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}


bool nb::OpenGl::OpenGLRender::init() noexcept
{
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

    PIXELFORMATDESCRIPTOR pfd = {};
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
        initFail("Failed to choose a multisample pixel format", nullptr);
        return false;
    }

    PIXELFORMATDESCRIPTOR pfdN;
    DescribePixelFormat(hdc, pixelFormatN, sizeof(pfdN), &pfdN);
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

    HGLRC modernContext = wglCreateContextAttribsARB(hdc, nullptr, contextAttribs);

    if (!modernContext)
    {
        initFail("Failed to create modern OpenGL context", tempContext);
        return false;
    }

    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(tempContext);

    if (!wglMakeCurrent(hdc, modernContext))
    {
        initFail("Failed to set the modern OpenGL context", modernContext);
        return false;
    }

    if (!gladLoadGL())
    {
        initFail("Failed to initialize GLAD", modernContext);
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

    ReleaseDC(dummyWindow, dummyHDC);
    DestroyWindow(dummyWindow);
    UnregisterClass(L"DummyWGLWindow", GetModuleHandle(nullptr));

    loadScene();
    return true;
}

void nb::OpenGl::OpenGLRender::initFail(std::string_view message, HGLRC context) noexcept
{
    Debug::debug(message);
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(context);
    ReleaseDC(hwnd, hdc);
}

void nb::OpenGl::OpenGLRender::loadScene() noexcept
{
    auto rm = nb::ResMan::ResourceManager::getInstance();
    shader = rm->getResource<nb::Renderer::Shader>("ADS.shader");

    auto mesh = rm->getResource<Renderer::Mesh>("untitled2_.obj");
    auto gizmo = rm->getResource<Renderer::Mesh>("Gizmo.obj");
    auto lumine = rm->getResource<Renderer::Mesh>("Tactical_Lumine (2).obj");
    auto wall = rm->getResource<Renderer::Mesh>("wall.obj");

    sceneGraph = Renderer::SceneGraph::getInstance();

    auto scene = sceneGraph->getScene();
    auto wtr = Math::translate(Math::Mat4<float>::identity(), {0.0f, 0.0f, 0.0f});

    Renderer::Transform at;

    auto n = std::make_shared<Renderer::ObjectNode>("Weapon 1", at, mesh, shader);
    auto g = std::make_shared<Renderer::ObjectNode>("Gizmo", at, gizmo, shader);

    Renderer::Transform t;
    t.translate = {0.0f, -10.0f, 0.0f};

    Renderer::Transform t2;
    t2.translate = {0.0f, 3.0f, 0.0f};
    t2.rotateX = Math::toRadians(90.0f);

    auto n5 = std::make_shared<Renderer::ObjectNode>("Lumine", t2, lumine, shader);

    Renderer::Transform wallTransform;
    wallTransform.translate = {0.0f, 0.0f, -15.0f};
    wallTransform.scale = {8.0f, 1.0f, 1.0f};
    wallTransform.rotateX = Math::toRadians(90.0f);

    auto nWall = std::make_shared<Renderer::ObjectNode>("Wall", wallTransform , wall, shader);


  
    auto box = rm->getResource<Renderer::Mesh>("box.obj");


    Renderer::Transform boxTransform;
    boxTransform.translate = {0.0f, 0.0f, 0.0f};
    boxTransform.scale = {4.0f, 4.0f, 4.0f};

    auto nBox = std::make_shared<Renderer::ObjectNode>("Box", boxTransform , box, shader);

    Renderer::Transform dirLightTransform;
    auto dirLight = std::make_shared<Renderer::DirectionalLight>(ambientColor, Math::Vector3<float>{1.0f, 1.0f, 1.0f}, Math::Vector3<float>{0.1f, 0.1f, 0.1f}, Math::Vector3<float>{-1.00f, 10.0f, -10.0f});
    auto dirLightNode = std::make_shared<Renderer::LightNode>("DirLight", dirLightTransform, dirLight);

    Renderer::Transform pointLightTransform;
    pointLightTransform.translate = {0.0f, 0.0f, 0.0f};
    auto pointLight = std::make_shared<Renderer::PointLight>(ambientColor, Math::Vector3<float>{1.0f, 1.0f, 1.0f}, Math::Vector3<float>{0.1f, 0.1f, 0.1f}, 
                pointLightTransform.translate, 1.0f, 0.0f, 0.0f , 1.0f);
    auto pointLightNode = std::make_shared<Renderer::LightNode>("PointLight", pointLightTransform, pointLight);

    auto hand = rm->getResource<Renderer::Mesh>("Hands.obj");
    Renderer::Transform handsTransform;
    auto hands = std::make_shared<Renderer::ObjectNode>("hands", handsTransform , hand, shader);


    //scene->addChildren(n);
    //scene->addChildren(g);
    scene->addChildren(n5);
    n5->addChildren(nBox);
    //scene->addChildren(nWall);
    //scene->addChildren(hands);
    //scene->addChildren(nBox);
    scene->addChildren(dirLightNode);
    dirLightNode->addChildren(pointLightNode);
    

}

void nb::OpenGl::OpenGLRender::visualizeLight(std::shared_ptr<Renderer::LightNode> node) const noexcept
{
    auto rm = ResMan::ResourceManager::getInstance();
    auto mesh = rm->getResource<Renderer::Mesh>("sphere.obj");

    auto lightVisulizeShader = rm->getResource<nb::Renderer::Shader>("lightVisulize.shader");

    mesh->uniforms.floatUniforms["time"] = Utils::Timer::timeElapsedSinceInit();
    mesh->uniforms.vec3Uniforms["viewPos"] = cam->getPosition();
    mesh->uniforms.mat4Uniforms["view"] = cam->getLookAt();
    mesh->uniforms.mat4Uniforms["proj"] = cam->getProjection();
    mesh->uniforms.mat4Uniforms["model"] = Math::translate(Math::Matrix<float, 4, 4>::identity(), node->getPosition() - Math::Vector3<float>{0.0f, 1.0f, 1.0f});

    mesh->draw(GL_TRIANGLES, lightVisulizeShader);

}

void nb::OpenGl::OpenGLRender::visualizeAabb(const Math::AABB3D &aabb, Math::Mat4<float> mat) const noexcept
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
        {{B.maxPoint.x, B.maxPoint.y, B.minPoint.z}, {}}};

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
        7, 4};

    Renderer::Mesh m(vertices, edges);
    m.uniforms.mat4Uniforms["model"] = Math::Mat4<float>::identity();
    m.uniforms.mat4Uniforms["view"] = cam->getLookAt();
    m.uniforms.mat4Uniforms["projection"] = cam->getProjection();

    m.draw(GL_LINES, aabbShader);
}




void nb::OpenGl::OpenGLRender::render()
{
    const int width = nb::Core::EngineSettings::getWidth();
    const int height = nb::Core::EngineSettings::getHeight();
    static bool reinit = false;

    /*if (width == 0 || height == 0)
    {
        reinit = true;
        return;
    }*/

    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto rm = nb::ResMan::ResourceManager::getInstance();


    // Получение камеры и матриц
    auto view = cam->getLookAt();
    auto proj = cam->getProjection();

    // === FBO BUFFER ===
    // init
    if (reinit)
    {
		delete postProcessFbo;
		postProcessFbo = new PostProcessFbo(width, height);
    }

    if(!postProcessFbo)
    {
        if (width != 0 && height != 0)
        {
            postProcessFbo = new PostProcessFbo(width, height);
        }
    }

    GLuint fboHeigth = postProcessFbo->getHeight();
    GLuint fboWidth = postProcessFbo->getWidth();

    static bool isWindowMinimizePrevFrame = false;

    if(fboWidth != width || fboHeigth != height)
    {
        if (width != 0 && height != 0)
        {
			delete postProcessFbo;
			postProcessFbo = new PostProcessFbo(width, height);
        }
    }


    postProcessFbo->bind();
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    


    // === SKYBOX ===
    static Renderer::Skybox sky;
    auto skyboxShader = rm->getResource<nb::Renderer::Shader>("skybox.shader");
    skyboxShader->setUniformInt("skybox", 0);
    skyboxShader->setUniformMat4("view", view);
    skyboxShader->setUniformMat4("projection", proj);
    sky.render(skyboxShader);

    // === SHADER И МАТРИЦЫ ===
    shader = rm->getResource<nb::Renderer::Shader>("ADS.shader");

    shader->setUniformMat4("model", model);
    shader->setUniformMat4("view", view);
    shader->setUniformMat4("proj", proj);

    MVP = model * view * proj;

    // === ОБНОВЛЕНИЕ СЦЕНЫ ===
    sceneGraph->getScene()->updateWorldTransform();

    // === ТЕКСТУРЫ ===
    if (!t) {
        t = new OpenGlTexture("C:\\rep\\Hex\\NewByte-Engine\\build\\Engine\\Debug\\res\\brick.png");
        tn = new OpenGlTexture("C:\\rep\\Hex\\NewByte-Engine\\build\\Engine\\Debug\\res\\brick_normal.png");
    }


    // === СБОР ИСТОЧНИКОВ СВЕТА ===
    std::vector<Renderer::LightNode> lights;
    std::stack<std::shared_ptr<Renderer::BaseNode>> nodeStack;
    nodeStack.push(sceneGraph->getScene());

    while (!nodeStack.empty()) {
        auto node = nodeStack.top();
        nodeStack.pop();

        for (auto& child : node->getChildrens())
            nodeStack.push(child);

        if (auto lightNode = std::dynamic_pointer_cast<Renderer::LightNode>(node))
            lights.push_back(*lightNode);
    }

    // === РЕНДЕР ===

    // lightpass
    Math::Mat4 lightProj = Math::ortho(-100.0f, 100.0f, -100.0f, 100.0f, -100.0f, 100.0f);
    Math::Mat4 lightView = Math::lookAt(
        Math::Vector3<float>{-2.0f, 4.0f, -1.0f}, 
        Math::Vector3<float>{0.0f, 0.0f, 0.0f}, 
        Math::Vector3<float>{0.0f, 1.0f, 0.0f}
    );

    Math::Mat4<float> lightSpaceMatrix = lightProj * lightView;

    Ref<Renderer::Shader> lightPassShader = rm->getResource<Renderer::Shader>("lightPass.shader");
    

    const size_t sizeOfDepthTexture = 1024;
    glViewport(0, 0, sizeOfDepthTexture, sizeOfDepthTexture);
    // i dont give a fuck why it eat all the memory
    // update: ALWAYS DELETE TEXTURE!!!
    OpenGl::DepthBuffer depthBuffer(sizeOfDepthTexture, sizeOfDepthTexture);

    depthBuffer.bind();

    countOfDraws = 0;
    nodeStack.push(sceneGraph->getScene());

    while (!nodeStack.empty()) {
        auto node = nodeStack.top();
        nodeStack.pop();

        for (auto& child : node->getChildrens())
            nodeStack.push(child);

        if (auto n = std::dynamic_pointer_cast<Renderer::ObjectNode>(node); n != nullptr)
        {
            n->mesh->uniforms.mat4Uniforms["lightSpaceMatrix"] = lightSpaceMatrix;
            n->mesh->uniforms.mat4Uniforms["model"] = n->getWorldTransform();
            n->mesh->draw(GL_TRIANGLES, lightPassShader);
        }
    }

    depthBuffer.unBind();

    postProcessFbo->bind();
    glViewport(0, 0, width, height);

    refreshPolygonMode();

    countOfDraws = 0;
    nodeStack.push(sceneGraph->getScene());

    while (!nodeStack.empty()) {
        auto node = nodeStack.top();
        nodeStack.pop();

        for (auto& child : node->getChildrens())
            nodeStack.push(child);

        renderNode(node, lights, depthBuffer, lightSpaceMatrix);
    }

    postProcessFbo->unBind();
    // === ПОСТПРОЦЕССИНГ ===
        glViewport(0, 0, width, height);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        std::vector<Renderer::Vertex> quadVertices = {
            {{-1.0f, 1.0f, 0.0f}, {}},
            {{-1.0f, -1.0f, 0.0f}, {}},
            {{1.0f, -1.0f, 0.0f}, {}},
            {{-1.0f, 1.0f, 0.0f}, {}},
            {{1.0f, -1.0f, 0.0f}, {}},
            {{1.0f, 1.0f, 0.0f}, {}}};

        quadVertices[0].textureCoodinates = {0.0f, 1.0f};
        quadVertices[1].textureCoodinates = {0.0f, 0.0f};
        quadVertices[2].textureCoodinates = {1.0f, 0.0f};
        quadVertices[3].textureCoodinates = {0.0f, 1.0f};
        quadVertices[4].textureCoodinates = {1.0f, 0.0f};
        quadVertices[5].textureCoodinates = {1.0f, 1.0f};

        static std::vector<unsigned int> edges =
            {
                0, 1, 2, 3, 4, 5};

        auto quadShader = rm->getResource<Renderer::Shader>("quadShader.shader");
        quadShader->setUniformInt("depthMap", 3);
        quadShader->setUniformVec2("screenSize", {(float)width, (float)height});
        Renderer::Mesh quad(quadVertices, edges);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, postProcessFbo->getQuadTexture());
        quad.draw(GL_TRIANGLES, quadShader);

        // === ПЕРЕКЛЮЧЕНИЕ БУФЕРОВ ===
        SwapBuffers(hdc);
}

void nb::OpenGl::OpenGLRender::renderNode(std::shared_ptr<Renderer::BaseNode> node
    , const std::vector<Renderer::LightNode>& lightNode
    , const OpenGl::DepthBuffer& depthBuffer
    , const Math::Mat4<float>& lightSpaceMatrix
) noexcept
{
    if (node == nullptr)
        return;

    if (auto n = std::dynamic_pointer_cast<Renderer::ObjectNode>(node); n != nullptr)
    {
        countOfDraws++;
        // n->mesh->uniforms.shader = shader;
        n->mesh->uniforms.intUniforms["ourTexture"] = 1;
        n->mesh->uniforms.intUniforms["textureNormal"] = 2;
        n->mesh->uniforms.vec3Uniforms["viewPos"] = cam->getPosition();
        n->mesh->uniforms.mat4Uniforms["view"] = cam->getLookAt();
        n->mesh->uniforms.mat4Uniforms["proj"] = cam->getProjection();
        n->mesh->uniforms.mat4Uniforms["model"] = n->getWorldTransform();
        n->mesh->uniforms.mat4Uniforms["lightSpaceMatrix"] = lightSpaceMatrix;
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, depthBuffer.getDepthMap());
        n->mesh->uniforms.intUniforms["shadowMap"] = 4;
        n->mesh->uniforms.intUniforms[Renderer::ShaderConstants::COUNT_OF_DIRECTIONLIGHT_UNIFORM_NAME.data()] = Renderer::DirectionalLight::getCountOfDirectionalLights();

        for(const auto& i : lightNode)
        {
            i.light->applyUniforms(shader);
        }

        if(n->getName() == "Gizmo")
        {
            n->mesh->uniforms.mat4Uniforms["model"] = gizmoModelMat;
        }

        t->bind(1);
        tn->bind(2);
        //else

        n->mesh->draw(GL_TRIANGLES, shader);

        if (!shouldVisualizeAabb)
        {
            visualizeAabb(n->mesh->getAabb3d(), n->getWorldTransform());
        }
    }
    else if(auto n = std::dynamic_pointer_cast<Renderer::LightNode>(node); n != nullptr)
    {
        if (!shouldVisualizeLight)
        {
            visualizeLight(n);
        }
    }
}

nb::OpenGl::OpenGLRender *nb::OpenGl::OpenGLRender::create(HWND hwnd)
{
    OpenGLRender *render = new OpenGLRender(hwnd);
    if (!render->init())
    {
        delete render;
        return nullptr;
    }

    return render;
}

void nb::OpenGl::OpenGLRender::drawTransformationElements(const Ref<Renderer::Mesh> mesh) noexcept
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
    auto rm = nb::ResMan::ResourceManager::getInstance();
    shader = rm->getResource<nb::Renderer::Shader>("ADS.shader");
}

void nb::OpenGl::OpenGLRender::applyDefaultModelFlat() noexcept
{
    auto rm = nb::ResMan::ResourceManager::getInstance();
    shader = rm->getResource<nb::Renderer::Shader>("vertFlat.shader");
}

void nb::OpenGl::OpenGLRender::applyDiffuseReflectionModel() noexcept
{
    auto rm = nb::ResMan::ResourceManager::getInstance();
    shader = rm->getResource<nb::Renderer::Shader>("DiffuseReflection.shader");

    shader->setUniformVec3("lightPos", {1.0f, 1.0f, 1.0f});
    shader->setUniformVec3("Kd", ambientColor);
    shader->setUniformVec3("Ld", {0.498f, 0.294f, 0.784f});
}

void nb::OpenGl::OpenGLRender::applyAmbientDiffuseSpecularModel(Renderer::Camera *cam) noexcept
{
    auto rm = nb::ResMan::ResourceManager::getInstance();
    shader = rm->getResource<nb::Renderer::Shader>("ADS.shader");
    shader->setUniformVec3("La", {1.f, 1.f, 1.f});
    shader->setUniformVec3("Ka", mat.ambient);

    shader->setUniformVec3("Ld", {1.f, 1.f, 1.f});
    shader->setUniformVec3("Kd", mat.diffuse);
    shader->setUniformVec3("LightPos", {1.2f, 1.0f, 2.0f});

    // shader->setUniformMat4("ViewDir", cam->getLookAt() );
    shader->setUniformVec3("Ls", {1.0f, 1.0f, 1.0f});
    shader->setUniformVec3("Ks", mat.specular);
    shader->setUniformFloat("shine", mat.shininess);

    shader->setUniformVec3("viewPos", cam->getPosition());
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
nb::Math::Vector3<float> nb::OpenGl::OpenGLRender::lightPos = {0.0f, 0.0f, 0.0f};
nb::Math::Mat4<float> nb::OpenGl::OpenGLRender::MVP = {};
Ref<nb::Renderer::Shader> nb::OpenGl::OpenGLRender::shader = nullptr;
nb::Math::Mat4<float> nb::OpenGl::OpenGLRender::model({{1.0f, 0.f, 0.f, 0.f},
                                                       {0.f, 1.0f, 0.f, 0.f},
                                                       {0.f, 0.f, 1.0f, 0.f},
                                                       {0.f, 0.f, 0.f, 1.0f}});

nb::Renderer::Material nb::OpenGl::OpenGLRender::mat;

//
nb::Math::Mat4<float> nb::OpenGl::OpenGLRender::gizmoModelMat;
