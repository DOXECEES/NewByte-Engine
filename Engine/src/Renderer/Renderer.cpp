// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Renderer.hpp"
#include "Renderer/Objects/Objects.hpp"
#include "Material.hpp"

#include <Renderer/Scene.hpp>
#include "Light.hpp"

namespace nb::Renderer
{
    Renderer::Renderer(HWND hwnd, nb::Core::GraphicsAPI apiType) noexcept
    {
        switch (apiType)
        {
        case nb::Core::GraphicsAPI::OPENGL:
            api = nb::OpenGl::OpenGLRender::create(hwnd);
            if (!api)
            {
                std::abort();
            }
            break;
        case nb::Core::GraphicsAPI::DIRECTX:
            NB_FALLTHROUGH;
        case nb::Core::GraphicsAPI::VULKAN:
            break;
        }

        loadSceneEcs();
        //loadScene();

        std::vector<Vertex> quadVertices = {
            {{-1.0f, 1.0f, 0.0f}, {}},
            {{-1.0f, -1.0f, 0.0f}, {}},
            {{1.0f, -1.0f, 0.0f}, {}},
            {{-1.0f, 1.0f, 0.0f}, {}},
            {{1.0f, -1.0f, 0.0f}, {}},
            {{1.0f, 1.0f, 0.0f}, {}}
        };

        quadVertices[0].textureCoordinates = { 0.0f, 1.0f };
        quadVertices[1].textureCoordinates = { 0.0f, 0.0f };
        quadVertices[2].textureCoordinates = { 1.0f, 0.0f };
        quadVertices[3].textureCoordinates = { 0.0f, 1.0f };
        quadVertices[4].textureCoordinates = { 1.0f, 0.0f };
        quadVertices[5].textureCoordinates = { 1.0f, 1.0f };

        std::vector<uint32> quadIndices = { 0, 1, 2, 3, 4, 5 };

        quadScreenMesh = createRef<Mesh>(quadVertices, quadIndices);
        contextMeshCache = createRef<OpenGl::OpenglContextMeshCache>();
    }

    void Renderer::onResize(uint32 width, uint32 heigth) noexcept
    {
        static uint32 prevWidth = 0;
        static uint32 prevHeigth = 0;

        if (width == 0 || heigth == 0)
        {
            return;
        }

        if (prevWidth == width && prevHeigth == heigth)
        {
            return;
        }

        prevWidth = width;
        prevHeigth = heigth;

        if (!t)
        {
            t = new OpenGl::OpenGlTexture("Assets\\res\\brick.png");
            tn = new OpenGl::OpenGlTexture("Assets\\res\\brick_normal.png");
        }

        mainFrameBuffer = api->createFrameBuffer(width, heigth);
        mainFrameBuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::COLOR);
        mainFrameBuffer->addRenderBufferAttachment(IFrameBuffer::RenderBufferAttachment::DEPTH_STENCIL);
        mainFrameBuffer->finalize();

        shadowFrameBuffer = api->createFrameBuffer(1024, 1024);
        shadowFrameBuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::DEPTH);
        shadowFrameBuffer->finalize();
        
        navigationalGizmoFrameBuffer = api->createFrameBuffer(400, 400);
        navigationalGizmoFrameBuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::COLOR);
        navigationalGizmoFrameBuffer->addRenderBufferAttachment(IFrameBuffer::RenderBufferAttachment::DEPTH_STENCIL);
        navigationalGizmoFrameBuffer->finalize();


        if (!albedo)
        {
            albedo      = std::make_shared<OpenGl::OpenGlTexture>("Assets\\res\\albedo (2).png"); 
            metal       = std::make_shared<OpenGl::OpenGlTexture>("Assets\\res\\metal (2).png");
            roughtness  = std::make_shared<OpenGl::OpenGlTexture>("Assets\\res\\rough (2).png");
            ao          = std::make_shared<OpenGl::OpenGlTexture>("Assets\\res\\ao (3).png");
            normal      = std::make_shared<OpenGl::OpenGlTexture>("Assets\\res\\normal.png");
        }


        if (!debugLightMesh)
        {
            debugLightMesh = PrimitiveGenerators::createSphere(0.2f, 16, 16);
            debugLightShader = ResMan::ResourceManager::getInstance()->getResource<Shader>("lightVisulize.shader");
        }

        isResourceLoaded = true;
    }


    void Renderer::render() noexcept
    {
        const int width = nb::Core::EngineSettings::getWidth();
        const int height = nb::Core::EngineSettings::getHeight();
        if (width <= 0 || height <= 0)
        {
            return;
        }

        auto rm = ResMan::ResourceManager::getInstance();

        onResize(width, height);

        auto& scene = nb::Scene::getInstance();
        auto& registry = scene.getRegistry();

        scene.updateAllTransforms();

        std::vector<Ecs::EntityID> lights;
        nbstl::Vector<RendererCommand> mainQueue;

        scene.traverseAll(
            [&](Ecs::EntityID entityId)
            {
                Ecs::Entity entity{entityId};

                if (registry.has<LightComponent>(entity))
                {
                    lights.push_back(entityId);
                }

                if (registry.has<MeshComponent>(entity))
                {
                    auto& meshComp = registry.get<MeshComponent>(entity);
                    auto& transform = registry.get<TransformComponent>(entity);

                    auto* meshPtr = meshComp.mesh.get();

                    Pipeline mainP = {};
                    mainP.shader = meshPtr->uniforms.shader;
                    mainP.polygonMode = polygonMode;

                    // Берем уже вычисленную в пункте 1 матрицу
                    meshPtr->uniforms.mat4Uniforms["model"] = transform.worldMatrix;

                    mainQueue.pushBack({meshPtr, api->getCache().getOrCreate(mainP)});
                }
            }
        );


        api->beginFrame();

        const float shadowSize = 1024.0f;
      
        api->setViewport({ 0, 0, shadowSize, shadowSize });
        api->bindFrameBuffer(shadowFrameBuffer);
        api->setClearColor(Colors::BROWN, 1.0f, 0);
        api->clear(false, true, false); 

        float size = 35.0f; 

        Math::Mat4 lightProj = Math::ortho(-size, size, -size, size, 0.1f, 75.0f);
        

        Math::Vector3<float> lightDir = { -1.0f, -0.2f, -0.2f };
        lightDir.normalize();
        float distance = 20.0f; 
        Math::Vector3<float> lightPos = -lightDir * distance; 


        Math::Mat4 lightView = Math::lookAt(
            registry.get<TransformComponent>(Ecs::Entity{lights[0]}).position,
            Math::Vector3<float>{ 0.0f, 0.0f, 0.0f },  
            Math::Vector3<float>{ 0.0f, 1.0f, 0.0f }   
        );

        
        //Math::Mat4<float> lightSpaceMatrix = lightProj * lightView;

        Ref<Shader> lightPassShader = rm->getResource<Shader>("lightPass.shader");

        Pipeline shadowP = {};
        shadowP.shader = lightPassShader;
        shadowP.polygonMode = PolygonMode::FULL;
        uint32 shadowPSO = api->getCache().getOrCreate(shadowP);

        for (auto& cmd : mainQueue) {
            lightPassShader->setUniformMat4("lightProj", lightProj);
            lightPassShader->setUniformMat4("lightView", lightView);

            lightPassShader->setUniformMat4("model", cmd.mesh->uniforms.mat4Uniforms["model"]);

            // Используем команду для теней: тот же меш, но другой пайплайн (шейдер)
            RendererCommand shadowCmd = { cmd.mesh, shadowPSO };
            api->drawMesh(shadowCmd);
        }

        api->bindDefaultFrameBuffer();
       




        api->bindFrameBuffer(mainFrameBuffer);
        api->setViewport({ 0, 0, (float)width, (float)height });
        api->setClearColor(Colors::BLACK, 1.0f, 0);
        api->clear(true, true, false); 

        auto view = cam->getLookAt();
        auto proj = cam->getProjection();


        static Skybox sky;
        auto skyboxShader = rm->getResource<nb::Renderer::Shader>("skybox.shader");
        skyboxShader->setUniformInt("skybox", 0);
        skyboxShader->setUniformMat4("view", view);
        skyboxShader->setUniformMat4("projection", proj);
        sky.render(skyboxShader);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, shadowFrameBuffer->getTexture(0));

        

       

        for (auto& cmd : mainQueue) 
        {
            auto& u = cmd.mesh->uniforms;
            PBRMaterial mat(cmd.mesh->uniforms.shader);
            mat.setAlbedoMap(albedo);
            mat.setNormalMap(normal);
            mat.setMetallicMap(metal);
            mat.setRoughnessMap(roughtness);
            mat.setAmbientOcclusionMap(ao);

            mat.setInt("albedoMap", 0);
            mat.setInt("normalMap", 1);
            mat.setInt("metallicMap", 2);
            mat.setInt("roughnessMap", 3);
            mat.setInt("aoMap", 4);

            mat.apply(api);

            u.vec3Uniforms["viewPos"] = cam->getPosition();
            u.mat4Uniforms["view"] = cam->getLookAt();
            u.mat4Uniforms["proj"] = cam->getProjection();
            //u.mat4Uniforms["lightSpaceMatrix"] = lightSpaceMatrix;

            u.mat4Uniforms["lightView"] = lightView;
            u.mat4Uniforms["lightProj"] = lightProj;
            u.intUniforms["shadowMap"] = 5;

            std::vector<PointLight> pointLightsStorage;
            std::vector<DirectionalLight> dirLightsStorage;

            for (auto lightEntityId : lights)
            {
                Ecs::Entity entity{lightEntityId};
                auto& light = registry.get<LightComponent>(entity);
                auto& lightTransform = registry.get<TransformComponent>(entity);

                if (light.type == LightType::DIRECTIONAL)
                {
                    dirLightsStorage.emplace_back(
                        light.ambient.asVec3(), light.diffuse.asVec3(), light.specular.asVec3(),
                        light.direction
                    );
                    dirLightsStorage.back().applyUniforms(u.shader);
                }
                else if (light.type == LightType::POINT)
                {
                    pointLightsStorage.emplace_back(
                        light.ambient.asVec3(), light.diffuse.asVec3(), light.specular.asVec3(),
                        lightTransform.position, light.constant, light.linear, light.quadratic, 1.0f
                    );
                    pointLightsStorage.back().applyUniforms(u.shader);
                }
            }


            u.intUniforms[ShaderConstants::COUNT_OF_DIRECTIONLIGHT_UNIFORM_NAME.data()] = dirLightsStorage.size();
            u.intUniforms[ShaderConstants::COUNT_OF_POINTLIGHT_UNIFORM_NAME.data()] = pointLightsStorage.size();

            // Биндим текстуры ( brick, normal и т.д.)
            //u.intUniforms["ourTexture"] = 1;
            //u.intUniforms["textureNormal"] = 2;
            //if (t) t->bind(1);
            //if (tn) tn->bind(2);
            

            api->drawMesh(cmd);
        }

        if (isDebugPassEnabled)
        {
            Pipeline debugP = {};
            debugP.shader = debugLightShader;
            debugP.polygonMode = PolygonMode::LINES; 
            debugP.isDepthTestEnable = true;
            debugP.isBlendEnable = false;

            uint32 debugPSO = api->getCache().getOrCreate(debugP);
            debugLightShader->use();
            debugLightShader->setUniformMat4("view", view);
            debugLightShader->setUniformMat4("proj", proj);

            for (auto lightId : lights)
            {
                const auto& lightTransform = registry.get<TransformComponent>(Ecs::Entity{lightId});
                const auto& lightComponent = registry.get<LightComponent>(Ecs::Entity{lightId});

                Math::Mat4 model = Math::translate(
                    Math::Mat4<float>::identity(), lightTransform.position
                );
                
                debugLightShader->setUniformMat4("model", model);
                debugLightShader->setUniformVec3("u_Color", lightComponent.ambient.asVec3());

                RendererCommand debugPassCommand { debugLightMesh.get(), debugPSO };
                api->drawMesh(debugPassCommand);
            }
        }

        if (ctx.handle)
        {
            blitToWindow(ctx, checkedTextureId);
        }

        renderNavigationalGizmo();


        api->bindDefaultFrameBuffer(); 
        api->setViewport({ 0, 0, (float)width, (float)height });
        api->setClearColor(Colors::WHITE, 1.0f, 0);
        api->clear(true, false, false); 

        auto quadShader = rm->getResource<Shader>("quadShader.shader");
        quadShader->setUniformInt("depthMap", 3); 
        quadShader->setUniformVec2("screenSize", { (float)width, (float)height });


        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, mainFrameBuffer->getTexture(0));

        Pipeline quadPipeline = {};
        quadPipeline.shader = std::move(quadShader);
        quadPipeline.polygonMode = PolygonMode::FULL;
        quadPipeline.isDepthTestEnable = false; 

        RendererCommand postCmd = { quadScreenMesh.get(), api->getCache().getOrCreate(quadPipeline) };
        api->drawMesh(postCmd);

        api->endFrame(); 
    }

    void Renderer::togglePolygonVisibilityMode(PolygonMode mode) const noexcept
    {
        switch (mode)
        {
        case PolygonMode::POINTS:
            api->setpolygonModePoints();
            break;
        case PolygonMode::LINES:
            api->setPolygonModeLines();
            break;
        case PolygonMode::FULL:
            api->setPolygonModeFull();
            break;
        default:
            Debug::debug("Unsupported polygon mode");
            break;
        }
    }

    void Renderer::setWireframeMode(bool flag) noexcept
    {
        if (flag)
        {
            polygonMode = PolygonMode::LINES;
        }
        else
        {
            polygonMode = PolygonMode::FULL;
        }
    }

    void Renderer::showVertexColor(bool flag) noexcept
    {

    }

    bool Renderer::isResourceReady() const noexcept
    {
        return isResourceLoaded;
    }

    SharedWindowContext Renderer::createSharedContextForWindow(HWND handle) noexcept
    {
        ctx = api->shareContext(handle);
        return ctx;
    }

    void Renderer::blitToWindow(const SharedWindowContext& out, uint32 textureId)
    {
        if (!api->setContext(out.hdc, out.hglrc))
        {
            return;
        }

        RECT rc;
        GetWindowRect(out.handle, &rc); 
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;

        api->setViewport({ 0.0f, 0.0f, (float)width, (float)height });
        api->setClearColor(Colors::BLUE, 1.0f, 0);
        api->clear(true, false, false);

        auto quadShader = ResMan::ResourceManager::getInstance()->getResource<Shader>("quadShader2.shader");
        quadShader->use();
        quadShader->setUniformInt("sceneTexture", 0);

        api->bindTexture(0, textureId);

        auto mesh = contextMeshCache->get(out.hglrc, quadScreenMesh.get());
        if (!mesh)
        {
            mesh = contextMeshCache->insertMesh(out.hglrc, quadScreenMesh);
        }

        Pipeline pipeline = {};
        pipeline.shader = std::move(quadShader);
        pipeline.isDepthTestEnable = false;
        pipeline.polygonMode = PolygonMode::FULL;
        uint32 pso = api->getCache().getOrCreate(pipeline);

    
        RendererCommand shadowCmd = { mesh->source.get(),  pso };
        api->drawContextMesh(*mesh, pso);

        SwapBuffers(out.hdc);
        api->setDefaultContext();
    }


    void Renderer::renderNavigationalGizmo() noexcept
    {
        api->bindFrameBuffer(navigationalGizmoFrameBuffer);

        api->setViewport({ 0,0,400,400 });
        api->setClearColor(Colors::GOLD, 1.0f, 0);
        api->clear(true, true, false);

        Math::Mat4<float> gizmoView = Math::lookAt(
            Math::Vector3<float>(0, 0, 3), // Позиция камеры индикатора (фиксирована)
            Math::Vector3<float>(0, 0, 0), // Куда смотрит (в центр, где оси)
            Math::Vector3<float>(0, 1, 0)  // Вектор "вверх"
        );


        Math::Mat4<float> cameraView = cam->getLookAt();
        cameraView = Math::inverse(cameraView);
        cameraView[3][0] = 0.0f;
        cameraView[3][1] = 0.0f;
        cameraView[3][2] = 0.0f;
        cameraView[3][3] = 1.0f;

        

        //gizmoView = Math::translate(Math::Mat4<float>(1.0f), Math::Vector3<float>(0, 0, -5.0f)) * gizmoView;

        // Ортографическая проекция, чтобы оси были ровными
        Math::Mat4<float> gizmoProj = Math::ortho(-1.5f, 1.5f, -1.5f, 1.5f, 0.1f, 10.0f);
        //Math::Mat4<float> gizmoProj = cam->getProjection();
        //Math::Mat4<float> gizemoModel = Math::scale(Math::Mat4<float>(1.0f), {6.5f, 6.5f, 6.5f});
        Math::Mat4<float> gizemoModel = Math::Mat4<float>::identity();
        //gizemoModel = Math::translate(gizemoModel, { 0.0f, 0.0f, -1.0f });

        gizemoModel = cameraView;
        auto gizmoMesh = nb::ResMan::ResourceManager::getInstance()
            ->getResource<nb::Renderer::Mesh>("Cube22.obj");
        

        auto gizmoShader = nb::ResMan::ResourceManager::getInstance()
            ->getResource<nb::Renderer::Shader>("gizmoShader.shader");

        Pipeline pipeline = {};
        pipeline.shader = gizmoShader;
        pipeline.polygonMode = PolygonMode::FULL;
        pipeline.isDepthTestEnable = true;
        pipeline.isBlendEnable = false;

        uint32 pso = api->getCache().getOrCreate(pipeline);
        gizmoShader->use();
        gizmoShader->setUniformMat4("model", gizemoModel);
        gizmoShader->setUniformMat4("view", cameraView);
        gizmoShader->setUniformMat4("projection", gizmoProj);

        {
            RendererCommand debugPassCommand{ gizmoMesh.get(), pso };
            api->drawMesh(debugPassCommand);
        }

        //api->bindDefaultFrameBuffer();
    }

    
    void Renderer::loadSceneEcs() noexcept
    {
        auto rm = ResMan::ResourceManager::getInstance();
        auto shader = rm->getResource<Shader>("ADS.shader");

        auto& scene = nb::Scene::getInstance();

        Math::Vector3<float> ambientColor{0.0f, 0.0f, 0.0f};


        

        nb::Node dirLight = scene.createNode();
        dirLight.setName("DirLight");

        dirLight.addComponent(
            LightComponent{
                LightType::DIRECTIONAL,
                Colors::BLACK,
                Colors::WHITE,
                Color::fromLinearRgb(0.1f, 0.1f, 0.1f),
                {-1.0f, -0.2f, -0.2f}
            }
        );


        nb::Node pointLight = scene.createNode(dirLight.getId());
        pointLight.setName("PointLight");

        auto& plTransform = pointLight.getComponent<TransformComponent>();
        plTransform.position = {-5.0f, 19.0f, -5.0f};
        plTransform.dirty = true;

        pointLight.addComponent(
            LightComponent{
                LightType::POINT,
                Colors::BLACK,
                Colors::WHITE,
                Color::fromLinearRgb(0.1f, 0.1f, 0.1f),
                {0.0f,0.0f,0.0f},
                1.0f,
                0.0f,
                0.0f,
                1.0f
            }
        );


        nb::Node pointLight2 = scene.createNode();
        pointLight2.setName("PointLight1");

        auto& pl2Transform = pointLight2.getComponent<TransformComponent>();
        pl2Transform.position = {5.0f, 0.0f, 5.0f};
        pl2Transform.dirty = true;

        pointLight2.addComponent(
            LightComponent{
                LightType::POINT,
                Colors::BLACK,
                Colors::WHITE,
                Color::fromLinearRgb(0.1f, 0.1f, 0.1f),
                {0.0f,0.0f,0.0f},
                1.0f,
                0.0f,
                0.0f,
                1.0f
            }
        );


        Ref<Mesh> cube = rm->getResource<Mesh>("Untitled.obj");

        nb::Node cubeNode = scene.createNode();
        cubeNode.setName("cube");

        auto& cubeTransform = cubeNode.getComponent<TransformComponent>();
        cubeTransform.position = {0.0f, 0.0f, 0.0f};
        cubeTransform.scale = {2.25f, 2.25f, 2.25f};
        cubeTransform.dirty = true;
        cube->uniforms.shader = shader;

        cubeNode.addComponent(MeshComponent{cube});


        Ref<Mesh> surf = rm->getResource<Mesh>("scene.obj");

        nb::Node surfNode = scene.createNode();
        surfNode.setName("scene");

        auto& surfTransform = surfNode.getComponent<TransformComponent>();
        surfTransform.position = {0.0f, 0.0f, 0.0f};
        surfTransform.dirty = true;
        surf->uniforms.shader = shader;

        surfNode.addComponent(MeshComponent{surf});
    }
}
