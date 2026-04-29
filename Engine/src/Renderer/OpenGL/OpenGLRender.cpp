// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include <Windows.h>
#include <memory>
#include "OpenGLRender.hpp"

#include "../Objects/Objects.hpp"
#include "Core.hpp"
#include "DepthBuffer.hpp"
#include "Manager/ResourceManager.hpp"
#include "Math/Matrix/Transformation.hpp"
#include "OpenGLCubemap.hpp"
#include "Renderer/Cubemap.hpp"
#include "Renderer/OpenGL/OpenGLTexture.hpp"


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
            //Error::ErrorManager::instance()
            //    .report(Error::Type::WARNING, "Unsupported polygon mode");
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
        wglMakeCurrent(this->hdc, this->hglrc); // Активируем главный контекст

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

    void OpenGLRender::releaseContext(const Renderer::SharedWindowContext& context) noexcept
    {
        if (!context.hglrc)
        {
            return;
        }

        //if (wglGetCurrentContext() == hglrc)
        //{
        //    wglMakeCurrent(NULL, NULL);
        //}

        if (!wglDeleteContext(context.hglrc))
        {
            nb::Error::ErrorManager::instance().report(
                nb::Error::Type::FATAL, "Failed to release Opengl context"
            );
        }

        ReleaseDC(context.handle, context.hdc);
    }

    bool OpenGLRender::setDefaultContext() noexcept
    {
        return wglMakeCurrent(this->hdc, this->hglrc);
    }


    void OpenGLRender::beginFrame() noexcept
    {
        setClearColor(nb::Colors::WHITE, 1.0f, 0);
        clear(true, true, false);
        
    }

    void OpenGLRender::endFrame() noexcept
    {
        SwapBuffers(hdc);
    }

    void OpenGLRender::drawMesh(Renderer::RendererCommand& command) noexcept
    {
        bindPipeline(command.pipeline);
        command.mesh->draw(GL_TRIANGLES, pipelineCache.getDesc(activePipeline).shader, command.material);
    }

    void nb::OpenGl::OpenGLRender::drawVertexless(Renderer::RendererCommand& command) noexcept
    {
        bindPipeline(command.pipeline);
        if (command.mesh != nullptr)
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::INFO, "Mesh passed to vertexless draw call");
        }
        glBindVertexArray(emptyVao);

        glDrawArrays(
            GL_TRIANGLES,
            0,
            command.vertexCount 
        );

        glBindVertexArray(0);

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

        if (pipeline.isCullingEnable)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK); 
        }
        else
        {
            glDisable(GL_CULL_FACE);
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

    Ref<Renderer::Texture> OpenGLRender::createTexture2d(const Renderer::TextureDescriptor& descriptor) noexcept
    {
        GLint internalFormat = GL_RGB;
        GLenum dataFormat = GL_RGB;
        GLenum dataType = GL_UNSIGNED_BYTE;

        switch (descriptor.format)
        {
            case Renderer::TextureFormat::RGB:
                internalFormat = GL_RGB;
                dataFormat = GL_RGB;
                dataType = GL_UNSIGNED_BYTE;
                break;
            case Renderer::TextureFormat::RGBA:
                internalFormat = GL_RGBA;
                dataFormat = GL_RGBA;
                dataType = GL_UNSIGNED_BYTE;
                break;
            case Renderer::TextureFormat::RGB16F:
                internalFormat = GL_RGB16F; 
                dataFormat = GL_RGB;        
                dataType = GL_FLOAT;        
                break;
        }


        return createRef<OpenGlTexture>(
            descriptor.width,
            descriptor.height,
            internalFormat,
            dataFormat,
            dataType,
            descriptor.data
        );
    }

    Ref<Renderer::Cubemap> OpenGLRender::bakeTextureIntoCubeMap(Ref<Renderer::Texture> texture2d) noexcept
    {
        glDisable(GL_CULL_FACE);

        Ref<OpenGLCubemap> cubemap = std::make_shared<OpenGLCubemap>(texture2d->getWidth(),GL_RGB16F);

        Math::Mat4<float> captureProjection = Math::projection(Math::toRadians(90.0f), 1.0f, 0.1f, 10.0f);
        Math::Mat4<float> captureViews[] = {
            Math::lookAt<float>({0,0,0}, { 1, 0, 0}, {0,-1, 0}), // +X
            Math::lookAt<float>({0,0,0}, {-1, 0, 0}, {0,-1, 0}), // -X
            Math::lookAt<float>({0,0,0}, { 0, 1, 0}, {0, 0, 1}), // +Y
            Math::lookAt<float>({0,0,0}, { 0,-1, 0}, {0, 0,-1}), // -Y
            Math::lookAt<float>({0,0,0}, { 0, 0, 1}, {0,-1, 0}), // +Z
            Math::lookAt<float>({0,0,0}, { 0, 0,-1}, {0,-1, 0})  // -Z
        };

        auto equiToCubeShader = ResMan::ResourceManager::getInstance()->getResource<Renderer::Shader>("equi_to_cube.shader"); 

        auto frameBuffer = std::dynamic_pointer_cast<FBO>(
            this->createFrameBuffer(texture2d->getWidth(), texture2d->getWidth())
        );
        frameBuffer->addRenderBufferAttachment(
            Renderer::IFrameBuffer::RenderBufferAttachment::DEPTH
        );
        frameBuffer->finalize();

        //shader
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture2d->getId());

        Ref<Renderer::Mesh> cube =
            nb::ResMan::ResourceManager::getInstance()->getResource<Renderer::Mesh>(
                "unit_cube.obj"
            );

        equiToCubeShader->setUniformMat4("projection", captureProjection);
        equiToCubeShader->setUniformInt("equirectangularMap", 0);

        setViewport({0,0,(float)texture2d->getWidth(),(float)texture2d->getWidth()});
        
        for (int i = 0; i < 6; ++i)
        {
            equiToCubeShader->setUniformMat4("view", captureViews[i]);

            frameBuffer->attachTextureId(
                cubemap->getId(), GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i
            );

            clear(true, true,false);
            cube->draw(GL_TRIANGLES, equiToCubeShader);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->getId());
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        glEnable(GL_CULL_FACE);

        return cubemap;
    }

    Ref<Renderer::Cubemap>
    OpenGLRender::bakeIrradiance(Ref<Renderer::Cubemap> enviromentCubemap) noexcept
    {
        glDisable(GL_CULL_FACE);

        Math::Mat4<float> captureProjection =
            Math::projection(Math::toRadians(90.0f), 1.0f, 0.1f, 10.0f);

        Math::Mat4<float> captureViews[] = {
            Math::lookAt<float>({0, 0, 0}, {1, 0, 0}, {0, -1, 0}),  // +X
            Math::lookAt<float>({0, 0, 0}, {-1, 0, 0}, {0, -1, 0}), // -X
            Math::lookAt<float>({0, 0, 0}, {0, 1, 0}, {0, 0, 1}),   // +Y
            Math::lookAt<float>({0, 0, 0}, {0, -1, 0}, {0, 0, -1}), // -Y
            Math::lookAt<float>({0, 0, 0}, {0, 0, 1}, {0, -1, 0}),  // +Z
            Math::lookAt<float>({0, 0, 0}, {0, 0, -1}, {0, -1, 0})  // -Z
        };
        uint32_t size = 32; 

        auto irradianceMap = createRef<OpenGl::OpenGLCubemap>(size, GL_RGB16F);

        auto irradianceShader =
            ResMan::ResourceManager::getInstance()->getResource<Renderer::Shader>(
                "irradiance.shader"
            );
        auto unitCube = 
            nb::ResMan::ResourceManager::getInstance()->getResource<Renderer::Mesh>(
                "unit_cube.obj"
            );

        auto equiToCubeShader =
            ResMan::ResourceManager::getInstance()->getResource<Renderer::Shader>(
                "equi_to_cube.shader"
            );

        auto frameBuffer = std::dynamic_pointer_cast<FBO>(
            this->createFrameBuffer(0,0)
        );
        
        frameBuffer->bind();

        irradianceShader->setUniformMat4("projection", captureProjection); 

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, enviromentCubemap->getId());
        irradianceShader->setUniformInt("environmentMap", 0);


        glViewport(0, 0, size, size);

        for (unsigned int i = 0; i < 6; ++i)
        {
            irradianceShader->setUniformMat4("view", captureViews[i]);
            frameBuffer->attachTextureId(
                irradianceMap->getId(), GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i
            );
                 
            clear(true, true, false);
            unitCube->draw(GL_TRIANGLES, irradianceShader);
        }


        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glEnable(GL_CULL_FACE);

        return irradianceMap;

    }

    Ref<Renderer::Cubemap> OpenGLRender::bakePrefilter(Ref<Renderer::Cubemap> envCubemap) noexcept
    {
        glDisable(GL_CULL_FACE);

        Math::Mat4<float> projection = Math::projection(Math::toRadians(90.0f), 1.0f, 0.1f, 10.0f);

        Math::Mat4<float> views[] = {
            Math::lookAt<float>({0, 0, 0}, {1, 0, 0}, {0, -1, 0}),
            Math::lookAt<float>({0, 0, 0}, {-1, 0, 0}, {0, -1, 0}),
            Math::lookAt<float>({0, 0, 0}, {0, 1, 0}, {0, 0, 1}),
            Math::lookAt<float>({0, 0, 0}, {0, -1, 0}, {0, 0, -1}),
            Math::lookAt<float>({0, 0, 0}, {0, 0, 1}, {0, -1, 0}),
            Math::lookAt<float>({0, 0, 0}, {0, 0, -1}, {0, -1, 0})
        };

        uint32_t baseSize = 512;
        uint32_t maxMipLevels = 5;

        auto map = createRef<OpenGl::OpenGLCubemap>();
        glBindTexture(GL_TEXTURE_CUBE_MAP, map->getId());

        // 🔥 ПОЛНАЯ АЛЛОКАЦИЯ ВСЕХ MIP УРОВНЕЙ
        for (uint32_t mip = 0; mip < maxMipLevels; ++mip)
        {
            uint32_t size = baseSize >> mip;

            for (uint32_t i = 0; i < 6; ++i)
            {
                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mip, GL_RGB16F, size, size, 0, GL_RGB,
                    GL_FLOAT, nullptr
                );
            }
        }

        // 🔒 Фикс уровней
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, maxMipLevels - 1);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        auto shader = ResMan::ResourceManager::getInstance()->getResource<Renderer::Shader>(
            "prefilter.shader"
        );

        auto cube =
            ResMan::ResourceManager::getInstance()->getResource<Renderer::Mesh>("unit_cube.obj");

        // --- FBO ---
        GLuint fbo, rbo;
        glGenFramebuffers(1, &fbo);
        glGenRenderbuffers(1, &rbo);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);

        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, baseSize, baseSize);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

        // --- входная кубмапа ---
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap->getId());

        //shader->bind();
        shader->setUniformInt("environmentMap", 0);
        shader->setUniformMat4("projection", projection);

        for (uint32_t mip = 0; mip < maxMipLevels; ++mip)
        {
            uint32_t size = baseSize >> mip;

            glViewport(0, 0, size, size);

            float roughness = (float)mip / (float)(maxMipLevels - 1);
            shader->setUniformFloat("roughness", roughness);

            // resize depth под mip
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);

            for (uint32_t i = 0; i < 6; ++i)
            {
                shader->setUniformMat4("view", views[i]);

                glFramebufferTexture2D(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    map->getId(), mip
                );

                // 🔍 проверка FBO
                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                {
                    printf("FBO ERROR\n");
                }

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                cube->draw(GL_TRIANGLES, shader);
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glDeleteRenderbuffers(1, &rbo);
        glDeleteFramebuffers(1, &fbo);
        glEnable(GL_CULL_FACE);

        return map;
    }

    Ref<Renderer::Texture> OpenGLRender::bakeBRDF() noexcept
    {
        glDisable(GL_CULL_FACE);

        uint32_t size = 512;

        uint32_t brdfLUT;
        glGenTextures(1, &brdfLUT);
        glBindTexture(GL_TEXTURE_2D, brdfLUT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, size, size, 0, GL_RG, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        uint32_t captureFBO;
        glGenFramebuffers(1, &captureFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUT, 0);

        uint32_t drawBuffers[] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, drawBuffers);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            int i = 0;
        }

        glViewport(0, 0, size, size);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        auto brdfShader =
            ResMan::ResourceManager::getInstance()->getResource<Renderer::Shader>("brdf.shader");
        //brdfShader->bind();
        brdfShader->use();

        static uint32_t quadVAO = 0;
        if (quadVAO == 0)
        {
            float quadVertices[] = {
                -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
            };
            uint32_t quadVBO;
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(
                1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))
            );
        }


        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // Рисуем 4 вершины
        glBindVertexArray(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &captureFBO);
        glEnable(GL_CULL_FACE);

        return createRef<OpenGlTexture>(brdfLUT, size, size);
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

    void OpenGLRender::setClearColor(const Color& color, float depthValue,  int32 stencilValue) noexcept
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

    glDeleteVertexArrays(1, &emptyVao);

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
    //nb::Error::ErrorManager::instance().report(nb::Error::Type::WARNING, message);
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
        ReleaseDC(dummyWindow, dummyHDC);
        initFail("Failed to set the pixel format", nullptr);
        return false;
    }

    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    if (!wglCreateContextAttribsARB)
    {
        ReleaseDC(dummyWindow, dummyHDC);
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
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); 
    
    glDepthFunc(GL_LESS);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
    glEnable(GL_BLEND);

    ReleaseDC(dummyWindow, dummyHDC);
    DestroyWindow(dummyWindow);
    UnregisterClass(L"DummyWGLWindow", GetModuleHandle(nullptr));
    glGenVertexArrays(1, &emptyVao);


    const GLubyte* renderer = glGetString(GL_RENDERER);

    const GLubyte* vendor = glGetString(GL_VENDOR);

    const GLubyte* version = glGetString(GL_VERSION);

    nb::Error::ErrorManager::instance()
        .report(nb::Error::Type::INFO, "GPU information")
        .with("Video Card", renderer)
        .with("Vendor", vendor)
        .with("OpenGL Version", version);
           

    

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

    Renderer::Mesh m(vertices, edges, "");
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
