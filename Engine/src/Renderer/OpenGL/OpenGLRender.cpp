#include "OpenGLRender.hpp"

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

   

    // During init, enable debug output
    glEnable              ( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( MessageCallback, 0 );



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
    glClearColor(0.45f, 0.12f, 0.75f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT); 

    std::vector<nb::OpenGl::VBO::Vertex> vert = {
        Math::Vector3<float>(-0.5f, -0.5f, 0.0f),
        Math::Vector3<float>(0.5f, -0.5f, 0.0f),
        Math::Vector3<float>(0.0f,  0.5f, 0.0f)
    };

    VertexArray vao;

    vao.linkData(vert, {0, 1, 2});
    
    auto rm = nb::ResMan::ResourceManager::getInstance();
    auto shader = rm->getResource<nb::Renderer::Shader>("vert.shader");
    shader->setUniformVec4("ourColor", {1.0f, 0.4f, 0.4f, 1.0f});


    shader->use();
    vao.bind();
    vao.draw();


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
