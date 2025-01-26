#include "OpenGLRender.hpp"

#include "../Objects/Objects.hpp"

nb::OpenGl::OpenGLRender::OpenGLRender(HWND _hwnd) noexcept
    : IRenderAPI(_hwnd), hdc(GetDC(hwnd))
{
}

nb::OpenGl::OpenGLRender::~OpenGLRender() noexcept
{
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
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    ReleaseDC(dummyWindow, dummyHDC);
    DestroyWindow(dummyWindow);
    UnregisterClass(L"DummyWGLWindow", GetModuleHandle(nullptr));

    auto rm = nb::ResMan::ResourceManager::getInstance();
    shader = rm->getResource<nb::Renderer::Shader>("ADS.shader");

    auto mesh = rm->getResource<Renderer::Mesh>("untitled2_.obj");
    auto mesh2 = rm->getResource<Renderer::Mesh>("dode.obj");
    auto meshCube = rm->getResource<Renderer::Mesh>("cube.obj");
    auto lumine = rm->getResource<Renderer::Mesh>("Tactical_Lumine.obj");
    //Renderer::SceneGraph::SceneNode n;
    //n.mesh = mesh;  
    //sceneGraph.addChild(n);
    sceneGraph = Renderer::SceneGraph::getInstance();
    auto scene = sceneGraph->getScene();
    auto wtr = Math::translate(Math::Mat4<float>::identity(), {0.f, 0.0f, 0.0f});
    //scene->setTransform(wtr);

    Renderer::Transform at;

    auto n = std::make_shared<Renderer::ObjectNode>("Weapon 1", at, mesh, shader);
    Renderer::Transform t;
    t.translate = {0.0f, -10.0f, 0.0f};
    //auto n2 = std::make_shared<Renderer::ObjectNode>("docehedron", t , mesh2, shader);
    //auto t2 = Math::translate(Math::Mat4<float>::identity(), {0.0f, 10.0f, 0.0f});
    //t2 = Math::rotate(Math::Mat4<float>::identity(), {1.0f, 0.0f, 0.0f}, 90);
    //t2 = Math::translate(t2, {0.0f, 20.0f, 0.0f});
    Renderer::Transform t2;
    t2.translate = {0.0f, 20.0f, 0.0f};
    t2.rotateX = 90.0f;
    //t.translate = {0.0f, -10.0f, 0.0f};

    //auto n3 = std::make_shared<Renderer::ObjectNode>("Weapon 2", t2, mesh, shader);
    //auto n4 = std::make_shared<Renderer::ObjectNode>("Cube", t2, meshCube, shader);
    auto n5 = std::make_shared<Renderer::ObjectNode>("Lumine", t2, lumine, shader);


    scene->addChildren(n);
    //scene->addChildren(n4);
    scene->addChildren(n5);

    //scene->addChildren(n2);
    //scene->getChildren(0)->addChildren(n3);


    return true;
}

void nb::OpenGl::OpenGLRender::initFail(std::string_view message, HGLRC context) noexcept
{
    Debug::debug(message);
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(context);
    ReleaseDC(hwnd, hdc);
}

void nb::OpenGl::OpenGLRender::render()
{
    glViewport(0, 0, nb::Core::EngineSettings::getWidth(), nb::Core::EngineSettings::getHeight());
    glClearColor(0.45f, 0.12f, 0.75f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LESS);

    auto rm = nb::ResMan::ResourceManager::getInstance();

    static Math::Vector3<float> color = {1.0f, 0.5f, 0.31f};

  
    // TEMP
    auto mesh = rm->getResource<Renderer::Mesh>("doce.obj");
    auto view = cam->getLookAt();
    
    auto proj = cam->getProjection();

    static Renderer::Skybox sky;
    auto skyboxShader = rm->getResource<nb::Renderer::Shader>("skybox.shader");

    if(isOrtho)
        proj = cam->getOrtho();
    else
        proj = cam->getProjection();

    skyboxShader->setUniformInt("skybox", 0);
    skyboxShader->setUniformMat4("view", view);
    skyboxShader->setUniformMat4("projection", proj);
    
    sky.render(skyboxShader);
    //

    //
    static OpenGl::OpenGlTexture t("C:\\rep\\Hex\\NewByte-Engine\\build\\Engine\\Debug\\res\\2.png");
    static float vel = 0.001f;
    vel += 0.001f;
    
    if(isLerp)
    {
        lerpMaterial();
        applyAmbientDiffuseSpecularModel(cam);
    }
    shader->setUniformMat4("model", model);
    shader->setUniformMat4("view", view);
    shader->setUniformMat4("proj", proj);

   
    MVP = model * view * proj;
    t.bind(0);

    sceneGraph->getScene()->updateWorldTransform();

    std::stack<std::shared_ptr<Renderer::BaseNode>> stk;
    stk.push(sceneGraph->getScene());

    while (!stk.empty())
    {

        std::shared_ptr<Renderer::BaseNode> top = stk.top();
        stk.pop();
        for (auto& i : top->getChildrens())
        {
            stk.push(i);
        }
        renderNode(top);
    }

    SwapBuffers(hdc);
}

void nb::OpenGl::OpenGLRender::renderNode(std::shared_ptr<Renderer::BaseNode> node) noexcept 
{
    if (node == nullptr) return;

    if (auto n = std::dynamic_pointer_cast<Renderer::ObjectNode>(node); n != nullptr)
    {
        for(auto& i : n->mt)
        {
            i->shader->setUniformMat4("view", cam->getLookAt());
            i->shader->setUniformMat4("proj", cam->getProjection());
            i->shader->setUniformMat4("model", n->getWorldTransform());
            i->shader->setUniformVec3("lightPos", {1.0f, 1.0f, 1.0f});
            i->shader->setUniformVec3("Kd", {1.0f,1.0f,1.0f});
            i->shader->setUniformVec3("Ld", {1.0f,1.0f,1.0f});
        }
        
        
        //n->mt[0]->applyMaterial();
       // n->mt[0]->shader->use();
        n->mesh->draw(GL_TRIANGLES, shader);

        // size_t indiciesOffset = 0;
        // size_t matIndex = 0;
        // for(const auto& i : n->mesh->meshes)
        // {
        //     n->mesh->VAO.bind();
        //     n->mesh->VAO.draw(i->indices.size(), GL_TRIANGLES, indiciesOffset);
        //     indiciesOffset += i->indices.size();

        //         // activate material
        //         if(i->material == Renderer::Material())
        //         {   
        //             n->mt[0]->applyMaterial();
        //             n->mt[0]->shader->use();
        //         }
        //         else
        //         {
        //             n->mt[1]->applyMaterial();
        //             n->mt[1]->shader->use();
        //         }
        // }
        //n->mesh->drawAabb();
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

void nb::OpenGl::OpenGLRender::rotate(float angle, const nb::Math::Vector3<float> &axis) noexcept
{
    model = Math::rotate(model, axis, Math::toRadians(angle));
}

void nb::OpenGl::OpenGLRender::drawTransformationElements(const Ref<Renderer::Mesh> mesh) noexcept
{
    //Math::AABB3D aabb = mesh->getAABB();
    //Math::Vector3<float> center = aabb.center();

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
    shader = rm->getResource<nb::Renderer::Shader>("vert.shader");
}

void nb::OpenGl::OpenGLRender::applyDefaultModelFlat() noexcept
{
    auto rm = nb::ResMan::ResourceManager::getInstance();
    shader = rm->getResource<nb::Renderer::Shader>("vertFlat.shader");
}

void nb::OpenGl::OpenGLRender::lerpMaterial() noexcept
{
    // Renderer::Material materials[] = {
    //     {{0.0215, 0.1745, 0.0215}, {0.07568, 0.61424, 0.07568}, {0.633, 0.727811, 0.633}, 0.6},  // emerald
    //     {{0.135, 0.2225, 0.1575}, {0.54, 0.89, 0.63}, {0.316228, 0.316228, 0.316228}, 0.1},        // jade
    //     {{0.05375, 0.05, 0.06625}, {0.18275, 0.17, 0.22525}, {0.332741, 0.328634, 0.346435}, 0.3}, // obsidian
    //     {{0.25, 0.20725, 0.20725}, {1, 0.829, 0.829}, {0.296648, 0.296648, 0.296648}, 0.088},     // pearl
    //     {{0.1745, 0.01175, 0.01175}, {0.61424, 0.04136, 0.04136}, {0.727811, 0.626959, 0.626959}, 0.6} // ruby
    // };



    // static float lerpAlpha = 0.0f;   
    // static int currentMaterial = 0;  
    // static auto lastTime = std::chrono::steady_clock::now();
    // int nextMaterialIndex = (currentMaterial + 1) % (sizeof(materials) / sizeof(materials[0]));
    // Renderer::Material nextMat = materials[nextMaterialIndex];
    
    // auto now = std::chrono::steady_clock::now();
    // std::chrono::duration<float> elapsedTime = now - lastTime;
    
    // if (elapsedTime.count() >= 1.0f) 
    // {
    //     lastTime = now;

    //     lerpAlpha += 0.1f; 

    //     if (lerpAlpha >= 1.0f)
    //     {
    //             currentMaterial = (currentMaterial + 1) % (sizeof(materials) / sizeof(materials[0]));
    //                 mat = materials[currentMaterial];

    //             lerpAlpha = 0.0f;  
    //     }
    //     mat.lerp(nextMat, lerpAlpha);
    // }

}

void nb::OpenGl::OpenGLRender::applyDiffuseReflectionModel() noexcept
{
    auto rm = nb::ResMan::ResourceManager::getInstance();
    shader = rm->getResource<nb::Renderer::Shader>("DiffuseReflection.shader");

    shader->setUniformVec3("lightPos", {1.0f, 1.0f, 1.0f});
    shader->setUniformVec3("Kd", ambientColor);
    shader->setUniformVec3("Ld", {0.498f, 0.294f, 0.784f});
}

void nb::OpenGl::OpenGLRender::applyAmbientDiffuseSpecularModel(Renderer::Camera* cam) noexcept
{
    auto rm = nb::ResMan::ResourceManager::getInstance();
    shader = rm->getResource<nb::Renderer::Shader>("ADS.shader");
    shader->setUniformVec3("La", {1.f,1.f, 1.f});
    shader->setUniformVec3("Ka", mat.ambient);

    shader->setUniformVec3("Ld", {1.f,1.f, 1.f});
    shader->setUniformVec3("Kd", mat.diffuse);
    shader->setUniformVec3("LightPos", {1.2f, 1.0f, 2.0f});

    // shader->setUniformMat4("ViewDir", cam->getLookAt() );
    shader->setUniformVec3("Ls", {1.0f, 1.0f, 1.0f});
    shader->setUniformVec3("Ks", mat.specular);
    shader->setUniformFloat("shine",mat.shininess);

    shader->setUniformVec3("viewPos", cam->getPosition());
}

nb::Math::Vector3<float> nb::OpenGl::OpenGLRender::ambientColor = {0.0f, 0.0f, 0.0f};
nb::Math::Vector3<float> nb::OpenGl::OpenGLRender::lightPos = {0.0f, 0.0f, 0.0f};
nb::Math::Vector3<float> nb::OpenGl::OpenGLRender::pos = {0.0f, 0.0f, 0.0f};
nb::Math::Mat4<float> nb::OpenGl::OpenGLRender::MVP = {};
Ref<nb::Renderer::Shader> nb::OpenGl::OpenGLRender::shader = nullptr;
nb::Math::Mat4<float> nb::OpenGl::OpenGLRender::model({{1.0f, 0.f, 0.f, 0.f},
                             {0.f, 1.0f, 0.f, 0.f},
                             {0.f, 0.f, 1.0f, 0.f},
                             {0.f, 0.f, 0.f, 1.0f}});

bool nb::OpenGl::OpenGLRender::isOrtho = false;
bool nb::OpenGl::OpenGLRender::isLerp = false;

nb::Renderer::Material nb::OpenGl::OpenGLRender::mat;