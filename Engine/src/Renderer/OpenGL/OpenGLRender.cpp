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
    glViewport(0, 0, 1920, 1080);
    glClearColor(0.45f, 0.12f, 0.75f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

    std::vector<nb::Renderer::Vertex> vert = {

    Math::Vector3<float>{1.0f, 1.0f, 1.0f},
    Math::Vector3<float>{1.0f, 1.0f, -1.0f},
    Math::Vector3<float>{1.0f, -1.0f, 1.0f},
    Math::Vector3<float>{1.0f, -1.0f, -1.0f},
    Math::Vector3<float>{-1.0f, 1.0f, 1.0f},
    Math::Vector3<float>{-1.0f, 1.0f, -1.0f},
    Math::Vector3<float>{-1.0f, -1.0f, 1.0f},
    Math::Vector3<float>{-1.0f, -1.0f, -1.0f},
    Math::Vector3<float>{0.0f, 1.618f, 1.0f},    
    Math::Vector3<float>{0.0f, 1.618f, -1.0f},
    Math::Vector3<float>{0.0f, -1.618f, 1.0f},
    Math::Vector3<float>{0.0f, -1.618f, -1.0f},
    Math::Vector3<float>{1.618f, 0.0f, 1.0f},
    Math::Vector3<float>{1.618f, 0.0f, -1.0f},
    Math::Vector3<float>{-1.618f, 0.0f, 1.0f},
    Math::Vector3<float>{-1.618f, 0.0f, -1.0f},
    Math::Vector3<float>{1.0f, 0.0f, 1.618f},
    Math::Vector3<float>{1.0f, 0.0f, -1.618f},
    Math::Vector3<float>{-1.0f, 0.0f, 1.618f},
    Math::Vector3<float>{-1.0f, 0.0f, -1.618f}


    };

        std::vector<unsigned int> indices;


    // Renderer::Mesh m(vert, {
    //         0, 8, 4, 9, 1,
    //         0, 1, 9, 5, 8,
    //         0, 8, 5, 6, 4,
    //         1, 0, 4, 7, 9,
    //         1, 9, 7, 10, 5,
    //         2, 3, 11, 10, 6,
    //         2, 6, 10, 7, 3,
    //         2, 3, 7, 8, 6,
    //         3, 7, 10, 11, 4,
    //         4, 11, 3, 2, 5,
    //         5, 2, 10, 11, 6,
    //         8, 5, 6, 10, 9
    // });

    vert.clear();
    indices.clear();

    const float PI = 3.14159265358979323846f;

    // Шаги углов
    float deltaU = 2.0f * PI / 32;
    float deltaV = 2.0f * PI / 16;
    float R = 2.0f;
    float r = 1.0f;

    // Генерация вершин
    for (int i = 0; i <= 32; ++i) {
        float u = i * deltaU;
        float cosU = cos(u);
        float sinU = sin(u);

        for (int j = 0; j <= 16; ++j) {
            float v = j * deltaV;
            float cosV = cos(v);
            float sinV = sin(v);

            Renderer::Vertex vertex({(R + r * cosV) * cosU,(R + r * cosV) * sinU,r * sinV});

            vert.push_back(vertex);
        }
    }

    // Генерация индексов
    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < 16; ++j) {
            int current = i * (16 + 1) + j;
            int next = (i + 1) * (16 + 1) + j;

            // Первый треугольник
            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);

            // Второй треугольник
            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }

    Renderer::Mesh m(vert, indices);

    auto rm = nb::ResMan::ResourceManager::getInstance();
    auto shader = rm->getResource<nb::Renderer::Shader>("vert.shader");

    static Math::Vector4<float> color = { 0.70f, 0.10f, 0.62f, 0.0f};


    shader->setUniformVec4("ourColor", color);
    Math::Mat4<float> model({
        {1.0f, 0.f,0.f,0.f},
        {0.f, 1.0f,0.f,0.f},
        {0.f, 0.f,1.0f,0.f},
        {0.f, 0.f,0.f,1.0f}
    });

    // Math::Mat4<float> view({
    //     {1.0f, 0.f,0.f,0.f},
    //     {0.f, 1.0f,0.f,0.f},
    //     {0.f, 0.f,1.0f,0.f},
    //     {0.f, 0.f,0.f,1.0f}
    // });
    auto view = this->cam->getLookAt();
    

    static float vel = 0.001f;
    vel += 0.001f;
    model = Math::scale(model, {5.0f, 5.0f , 5.0f});
    
    auto proj = Math::projection(Math::toRadians(45.0f), 
        //(float)nb::Core::EngineSettings::getWidth() / (float)nb::Core::EngineSettings::getHeight(),
        1920.0f/1080.0f,
        1.0f,
        100.0f);

    shader->setUniformMat4("model", model);
    shader->setUniformMat4("view", view);
    shader->setUniformMat4("proj", proj);

    // for (int i = 0; i < 16; i++)
    // {
    //     float a = *(model.valuePtr() + i);
    // }

    shader->use();
    m.draw();
    


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
