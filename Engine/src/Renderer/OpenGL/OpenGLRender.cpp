#include "OpenGLRender.hpp"

#include "../Objects/Objects.hpp"

nb::OpenGl::OpenGLRender::OpenGLRender(HWND _hwnd) noexcept
    : IRenderAPI(_hwnd)
    , hdc(GetDC(hwnd))
{

}

nb::OpenGl::OpenGLRender::~OpenGLRender() noexcept
{
    wglMakeCurrent(nullptr, nullptr);
    ReleaseDC(hwnd, hdc);
}

 void GLAPIENTRY
    MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}

bool nb::OpenGl::OpenGLRender::init() noexcept
{
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelFormat, &pfd);

    HGLRC tempContext = wglCreateContext(hdc);
    wglMakeCurrent(hdc, tempContext);

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
        0 
    };

    HGLRC modernContext = wglCreateContextAttribsARB(hdc, nullptr, contextAttribs);

    if (!modernContext)
    {
        initFail("Failed to create modern OpenGL context", tempContext);
        return false;
    }

    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(tempContext);
    
    if (!wglMakeCurrent(hdc, modernContext)) {
        initFail("Failed to set the modern OpenGL context", modernContext);
        return false;
    }

    if (!gladLoadGL())
    {
        initFail("Failed to initialize GLAD", modernContext);
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable              ( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( MessageCallback, 0 );
    //glShadeModel(GL_SMOOTH);


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

    auto rm = nb::ResMan::ResourceManager::getInstance();
    auto shader = rm->getResource<nb::Renderer::Shader>("vert.shader");

    static Math::Vector3<float> color = { 1.0f, 0.5f, 0.31f};

    shader->setUniformVec3("lightPos", {1.0f, 0.0f, 0.0f});
    shader->setUniformVec3("viewPos", cam->getPosition());

    shader->setUniformVec3("objectColor", color);
    shader->setUniformVec3("lightColor", ambientColor);

    Math::Mat4<float> model({
        {1.0f, 0.f,0.f,0.f},
        {0.f, 1.0f,0.f,0.f},
        {0.f, 0.f,1.0f,0.f},
        {0.f, 0.f,0.f,1.0f}
    });

    auto mesh = rm->getResource<Renderer::Mesh>("untitled2_.obj");
    //auto mesh = nb::Renderer::Mesh(vert, cubeIndices);
    // if(m == nullptr)
    // m = createRef<nb::Renderer::Mesh>(vert, cubeIndices);

    auto view = cam->getLookAt();
    auto proj = cam->getProjection();

    static float vel = 0.001f;
    vel += 0.001f;
    model = Math::translate(model, {0.0f, 10.0f , 0.0f});

    shader->setUniformMat4("model", model);
    shader->setUniformMat4("view", view);
    shader->setUniformMat4("projection", proj);

    shader->use();
    mesh->draw();
    


    // for(const auto& obj : scene)
    // {
    //     obj->render();
    // }



    SwapBuffers(hdc); 
}

nb::OpenGl::OpenGLRender *nb::OpenGl::OpenGLRender::create(HWND hwnd)
{
    OpenGLRender *render = new OpenGLRender(hwnd);
    if(!render->init())
    {
        delete render;
        return nullptr;
    }

    return render;
}


nb::Math::Vector3<float> nb::OpenGl::OpenGLRender::ambientColor = { 0.0f, 0.0f, 0.0f};