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
    auto lumine = rm->getResource<Renderer::Mesh>("Tactical_Lumine.obj");
    
    sceneGraph = Renderer::SceneGraph::getInstance();
    
    auto scene = sceneGraph->getScene();
    auto wtr = Math::translate(Math::Mat4<float>::identity(), {0.f, 0.0f, 0.0f});

    Renderer::Transform at;

    auto n = std::make_shared<Renderer::ObjectNode>("Weapon 1", at, mesh, shader);
    auto g = std::make_shared<Renderer::ObjectNode>("Gizmo", at, gizmo, shader);

    Renderer::Transform t;
    t.translate = {0.0f, -10.0f, 0.0f};
    
    Renderer::Transform t2;
    t2.translate = {0.0f, 20.0f, 0.0f};
    t2.rotateX = Math::toRadians(90.0f);

    auto n5 = std::make_shared<Renderer::ObjectNode>("Lumine", t2, lumine, shader);

    scene->addChildren(n);
    scene->addChildren(g);
    scene->addChildren(n5);
}

void nb::OpenGl::OpenGLRender::render()
{

    glViewport(0, 0, nb::Core::EngineSettings::getWidth(), nb::Core::EngineSettings::getHeight());
    glClearColor(0.45f, 0.12f, 0.75f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LESS);

    auto rm = nb::ResMan::ResourceManager::getInstance();

    auto view = cam->getLookAt();
    auto proj = cam->getProjection();

    static Renderer::Skybox sky;
    auto skyboxShader = rm->getResource<nb::Renderer::Shader>("skybox.shader");

    proj = cam->getProjection();

    skyboxShader->setUniformInt("skybox", 0);
    skyboxShader->setUniformMat4("view", view);
    skyboxShader->setUniformMat4("projection", proj);
    
    sky.render(skyboxShader);
    //

    //

    shader->setUniformMat4("model", model);
    shader->setUniformMat4("view", view);
    shader->setUniformMat4("proj", proj);

    MVP = model * view * proj;

    sceneGraph->getScene()->updateWorldTransform();


    // GLuint fbo;
    // glGenFramebuffers(1, &fbo);
    // glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // //
    // GLuint bufferPositions;
    // GLuint bufferNormals;
    // GLuint bufferColor;

    // glGenTextures(1, &bufferPositions);
    // glBindTexture(GL_TEXTURE_2D, bufferPositions);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, nb::Core::EngineSettings::getWidth(), nb::Core::EngineSettings::getHeight(), 0, GL_RGBA, GL_FLOAT, NULL);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferPositions, 0);


    // glGenTextures(1, &bufferNormals);
    // glBindTexture(GL_TEXTURE_2D, bufferNormals);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, nb::Core::EngineSettings::getWidth(), nb::Core::EngineSettings::getHeight(), 0, GL_RGBA, GL_FLOAT, NULL);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bufferNormals, 0);

    // glGenTextures(1, &bufferColor);
    // glBindTexture(GL_TEXTURE_2D, bufferColor);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, nb::Core::EngineSettings::getWidth(), nb::Core::EngineSettings::getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, bufferColor, 0);

    // unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    // glDrawBuffers(3, attachments);

    // unsigned int rboDepth;
    // glGenRenderbuffers(1, &rboDepth);
    // glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, nb::Core::EngineSettings::getWidth(), nb::Core::EngineSettings::getHeight());
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // // finally check if framebuffer is complete
    // if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    //     //std::cout << "Framebuffer not complete!" << std::endl;
    //     {

    //     }
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // auto lightShader = rm->getResource<nb::Renderer::Shader>("lightShader.shader");

    // lightShader->setUniformInt("gPosition", 0);
    // lightShader->setUniformInt("gNormal", 1);
    // lightShader->setUniformInt("gAlbedoSpec", 2);


    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, bufferPositions);
    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, bufferNormals);
    // glActiveTexture(GL_TEXTURE2);
    // glBindTexture(GL_TEXTURE_2D, bufferColor);

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


Ref<nb::Renderer::Mesh> generateTranslateGizmo()
{
    const float height = 5;
    const float radius = 2.0f;
    const int segmentsCount = 32;
    // bottom + surface + caps
    std::vector<nb::Renderer::Vertex> vert((segmentsCount + 1) *2 + 1);

//     // cylinder
    float thetaStep = 2.0f * nb::Math::Constants::PI / segmentsCount;

    for (int i = 0; i <= segmentsCount; i++)
    {
        float theta = i * thetaStep;
        const float sinU = sin(theta);
        const float cosU = cos(theta);

        nb::Renderer::Vertex v1 = {{radius * cosU, radius * sinU, height / 2}};
        nb::Renderer::Vertex v2 = {{radius * cosU, radius * sinU, -height / 2}};
        vert[i] = (v1);
        vert[segmentsCount + 1 + i] = (v2);
    }

    int k1 = 0;                        
    int k2 = segmentsCount + 1;  
    std::vector<GLuint> ind; 

    for(int i = 0; i < segmentsCount * 6; i += 6, ++k1, ++k2)
    {

        ind.push_back(k1);
        ind.push_back(k1 + 1);
        ind.push_back(k2);

        ind.push_back(k2);
        ind.push_back(k1 + 1);
        ind.push_back(k2 + 1);
    }

    vert.push_back({{0.0f, 0.0f, -height / 2}});
    GLuint bottomCenterIndex = vert.size() - 1;

    for (int i = segmentsCount + 1; i < (segmentsCount+1)*2; i++)
    {
        ind.push_back(i);
        ind.push_back(bottomCenterIndex);
        ind.push_back(i + 1);
    }

    // caps
    const float capRadius = radius + 1;
    const float capOffset = height / 2.0f;
    const float capHeight = 2.0f;
    const int capSegmentCount = 32;

    float capThetaStep = 2.0f * nb::Math::Constants::PI / capSegmentCount;

    for (int i = 0; i <= capSegmentCount; i++)
    {
        float theta = capThetaStep * i;
        const float sinU = sin(theta);
        const float cosU = cos(theta);

        nb::Renderer::Vertex v = {{capRadius * cosU, capRadius * sinU, capOffset}};
        vert.push_back(v);
    }

    vert.push_back({{0.0f, 0.0f, capOffset}});
    GLuint bottomCenterCapIndex = vert.size() - 1;
    
    // for (int i = 0; i < (capSegmentCount + 1) * 2; i++)
    // {
    //     ind.push_back(i);
    //     ind.push_back(bottomCenterCapIndex);
    //     ind.push_back(i + 1);
    // }


    vert.push_back({{0.0f, 0.0f, capOffset + capHeight}});
    GLuint topCenterCapIndex = vert.size() - 1;




    return createRef<nb::Renderer::Mesh>(vert, ind);

}


void nb::OpenGl::OpenGLRender::renderNode(std::shared_ptr<Renderer::BaseNode> node) noexcept 
{
    if (node == nullptr) return;

    if (auto n = std::dynamic_pointer_cast<Renderer::ObjectNode>(node); n != nullptr)
    {
        /// bullshit rewrite 
        for(auto& i : n->mt)
        {
            i->shader = shader;
            i->mat4Uniforms["view"] = cam->getLookAt();
            i->mat4Uniforms["proj"] = cam->getProjection();
            i->mat4Uniforms["model"] = n->getWorldTransform();
            i->vec3Uniforms["light[0].LightPos"] = {1.0f, 1.0f, 1.0f};
            //i->vec3Uniforms["light[0].La"] = {0.0f, 0.0f, 1.0f};
            i->vec3Uniforms["light[0].Ld"] = {1.0f, 1.0f, 1.0f};
            i->vec3Uniforms["light[1].LightPos"] = {-1.00f, 10.0f, -10.0f};
            i->vec3Uniforms["light[1].La"] = {0.1f, 0.1f, 0.1f};
            i->vec3Uniforms["light[1].Ld"] = {1.0f, 1.0f, 1.0f};
        }
        //auto m = generateTranslateGizmo();
        //m->draw(GL_TRIANGLES, shader);

        n->mt[0]->applyMaterial();
        n->mesh->draw(GL_TRIANGLES, shader);
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
nb::Math::Mat4<float> nb::OpenGl::OpenGLRender::MVP = {};
Ref<nb::Renderer::Shader> nb::OpenGl::OpenGLRender::shader = nullptr;
nb::Math::Mat4<float> nb::OpenGl::OpenGLRender::model({{1.0f, 0.f, 0.f, 0.f},
                             {0.f, 1.0f, 0.f, 0.f},
                             {0.f, 0.f, 1.0f, 0.f},
                             {0.f, 0.f, 0.f, 1.0f}});


nb::Renderer::Material nb::OpenGl::OpenGLRender::mat;