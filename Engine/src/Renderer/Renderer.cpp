// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Renderer.hpp"
#include "Manager/ResourceManager.hpp"
#include "Math/Matrix/Transformation.hpp"
#include "Math/Vector3.hpp"
#include "Renderer/Objects/Objects.hpp"
#include "Material.hpp"
#include "Resources/IhdrResource.hpp"

#include <Renderer/Scene.hpp>
#include "Light.hpp"

#include "Serialize/JsonArchive.hpp"

#include "Scripting/ScriptComponent.hpp"
#include "Physics/Physics.hpp"

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

        quadScreenMesh = createRef<Mesh>(quadVertices, quadIndices, "");
        contextMeshCache = createRef<OpenGl::OpenglContextMeshCache>();
    
        gizmoCtx.render = [&](const tinygizmo::geometry_mesh& mesh)
        {
            if (mesh.vertices.empty())
            {
                return;
            }

            std::vector<nb::Renderer::Vertex> nbVertices;
            nbVertices.reserve(mesh.vertices.size());

            for (const auto& gv : mesh.vertices)
            {
                nb::Renderer::Vertex v;
                v.position = {gv.position.x, gv.position.y, gv.position.z};
                v.normal = {gv.normal.x, gv.normal.y, gv.normal.z};
                v.color = {gv.color.x, gv.color.y, gv.color.z};

                v.textureCoordinates = {0.0f, 0.0f};
                v.tangent = {0.0f, 0.0f, 0.0f, 1.0f};

                nbVertices.push_back(v);
            }

            std::vector<uint32_t> nbIndices;
            nbIndices.reserve(mesh.triangles.size() * 3);

            for (const auto& tri : mesh.triangles)
            {
                nbIndices.push_back(tri.x);
                nbIndices.push_back(tri.y);
                nbIndices.push_back(tri.z);
            }
            Mesh m = Mesh(nbVertices, nbIndices, "");

            auto sh = nb::ResMan::ResourceManager::getInstance()->getResource<Shader>(
                "gizmoShader.shader"
            );

            Pipeline gridPipeline{.shader = sh, .isDepthTestEnable = false, .isBlendEnable = true};

            uint32 gridPSO = api->getCache().getOrCreate(gridPipeline);

            RendererCommand gridRenderCommand{.mesh = &m, .pipeline = gridPSO};

            sh->setUniformMat4("uViewProj", cam->getLookAt() * cam->getProjection());

            api->drawMesh(gridRenderCommand);
        };
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
            albedo      = std::make_shared<OpenGl::OpenGlTexture>("Assets\\res\\beige_wall_001_diff_1k.png"); 
            metal       = std::make_shared<OpenGl::OpenGlTexture>("Assets\\res\\metal (2).png");
            roughtness  = std::make_shared<OpenGl::OpenGlTexture>("Assets\\res\\rough (2).png");
            ao          = std::make_shared<OpenGl::OpenGlTexture>("Assets\\res\\ao (3).png");
            normal      = std::make_shared<OpenGl::OpenGlTexture>("Assets\\res\\beige_wall_001_nor_gl_1k.png");
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
        auto shader = rm->getResource<Shader>("ADS.shader");

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
                    mainP.shader = shader;
                    mainP.polygonMode = polygonMode;

                    // Берем уже вычисленную в пункте 1 матрицу
                    meshPtr->uniforms.mat4Uniforms["model"] = transform.worldMatrix;

                    mainQueue.pushBack({
                        .mesh     = meshPtr,
                        .material = meshComp.material.get(),
                        .pipeline = api->getCache().getOrCreate(mainP)
                    });
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

            RendererCommand shadowCmd = { .mesh = cmd.mesh, .pipeline = shadowPSO };
            api->drawMesh(shadowCmd);
        }

        api->bindDefaultFrameBuffer();
       




        api->bindFrameBuffer(mainFrameBuffer);
        api->setViewport({ 0, 0, (float)width, (float)height });
        api->setClearColor(Colors::BLACK, 1.0f, 0);
        api->clear(true, true, false); 

        auto view = cam->getLookAt();
        auto proj = cam->getProjection();


        auto ibl = nb::ResMan::ResourceManager::getInstance()->getResource<Resource::IhdrResource>(
            "Assets/res/lobby.hdr"
        );


        static Skybox sky;
        auto skyboxShader = rm->getResource<nb::Renderer::Shader>("skybox.shader");
        skyboxShader->setUniformInt("skybox", 0);
        skyboxShader->setUniformMat4("view", view);
        skyboxShader->setUniformMat4("projection", proj);
        sky.bindCubemap(ibl->getPrefilterCubemap());
        sky.render(skyboxShader);

        //glActiveTexture(GL_TEXTURE5);
        //glBindTexture(GL_TEXTURE_2D, shadowFrameBuffer->getTexture(0));


        //


        if (isShowGridEnabled)
        {
            auto gridShader = rm->getResource<nb::Renderer::Shader>("infinite_grid.shader");

            Pipeline gridPipeline
            {
                .shader = gridShader,
                .isDepthTestEnable = false,
                .isBlendEnable = true
            };

            uint32 gridPSO = api->getCache().getOrCreate(gridPipeline);

            RendererCommand gridRenderCommand
            {
                .mesh = nullptr,
                .pipeline = gridPSO,
                .vertexCount = 6
            };


            gridShader->setUniformVec3("uCameraWorldPosition", cam->getPosition());
            gridShader->setUniformMat4("uViewProjection", cam->getLookAt() * cam->getProjection());

            api->drawVertexless(gridRenderCommand);
        }

        
        //

        for (auto& cmd : mainQueue) 
        {
            auto& u = cmd.mesh->uniforms;
            cmd.mesh->uniforms.shader = shader;

            for (int i = 0; i < 8; i++)
            {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            }


           if (cmd.material)
            {
                auto& material = cmd.material;
                auto  shader   = material->getShader();

                // 0, 1, 2 привязываются внутри material->bind
                material->bind(api);

                // 3. Shadow Map (в шейдере binding = 3)
                api->bindTexture(3, shadowFrameBuffer->getTexture());

                // 4. Irradiance (в шейдере binding = 4)
                // Важно: если api->bindTexture не поддерживает CubeMap,
                // используй glActiveTexture + glBindTexture вручную
                
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->getIrradianceCubemap()->getId());
                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::FATAL, "Irradiance")
                    .with("id", ibl->getIrradianceCubemap()->getId());

                // 5. Prefilter (в шейдере binding = 5)
                glActiveTexture(GL_TEXTURE5);
                glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->getPrefilterCubemap()->getId());

                // 6. BrdfLUT (в шейдере binding = 6)
                api->bindTexture(6, ibl->getBrdfTexture()->getId());

                shader->setUniformVec3("u_CameraPos", cam->getPosition());
            }
            else
            {
                PBRMaterial mat(shader);
                mat.setAlbedoMap(albedo);
                mat.setNormalMap(normal);
                mat.setMetallicMap(metal);
                mat.setRoughnessMap(roughtness);
                mat.setAmbientOcclusionMap(ao);

                mat.setInt("albedoMap", 0);
                mat.setInt("normalMap", 1);
                mat.setInt("metallicMap", 2);
                //mat.setInt("roughnessMap", 3);
                //mat.setInt("aoMap", 4);

                mat.apply(api);
                api->bindTexture(3, shadowFrameBuffer->getTexture());
            
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->getIrradianceCubemap()->getId());

                glActiveTexture(GL_TEXTURE5);
                glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->getPrefilterCubemap()->getId());

                api->bindTexture(6, ibl->getBrdfTexture()->getId());

            }
            

            u.vec3Uniforms["viewPos"] = cam->getPosition();
            u.mat4Uniforms["view"] = cam->getLookAt();
            u.mat4Uniforms["proj"] = cam->getProjection();
            //u.mat4Uniforms["lightSpaceMatrix"] = lightSpaceMatrix;

            u.mat4Uniforms["lightView"] = lightView;
            u.mat4Uniforms["lightProj"] = lightProj;
            //u.intUniforms["shadowMap"] = 6;

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

                RendererCommand debugPassCommand { .mesh = debugLightMesh.get(), .pipeline = debugPSO };
                api->drawMesh(debugPassCommand);
            }
        }
        if (isBoundingBoxVisualizationEnabled)
        {
            Ref<Shader> aabbShader = rm->getResource<Shader>("aabb.shader");
            
            Ref<Mesh> unitCubeMesh = rm->getResource<Mesh>("unit_cube.obj");
            
            Pipeline aabbVisualisation = {};
            aabbVisualisation.shader = aabbShader;
            aabbVisualisation.polygonMode = PolygonMode::LINES;
            aabbVisualisation.isDepthTestEnable = false;

            uint32 aabbPSO = api->getCache().getOrCreate(aabbVisualisation);

            aabbShader->use();
            aabbShader->setUniformMat4("view", view);
            aabbShader->setUniformMat4("projection", proj);

            for (auto& cmd : mainQueue)
            {
                Math::AABB3D localAabb = cmd.mesh->getAabb3d();

                Math::AABB3D worldAabb = Math::AABB3D::recalculateAabb3dByModelMatrix(
                    localAabb, cmd.mesh->uniforms.mat4Uniforms["model"]
                );
                auto model = Math::Mat4<float>::identity();
                model = Math::scale(model, worldAabb.size() * 0.5f);
                model = Math::translate(model, worldAabb.center());

                aabbShader->setUniformMat4("model", model);

                RendererCommand command{ .mesh = unitCubeMesh.get(), .pipeline = aabbPSO};
                api->drawMesh(command);
            }

        }

        if (isBVHVisualizationEnabled)
        {
            auto bvh = Scene::getInstance().getBvh();
            if (!bvh->items.empty())
            {
                Ref<Shader> aabbShader = rm->getResource<Shader>("aabb.shader");

                Ref<Mesh> unitCubeMesh = rm->getResource<Mesh>("unit_cube.obj");

                Pipeline aabbVisualisation = {};
                aabbVisualisation.shader = aabbShader;
                aabbVisualisation.polygonMode = PolygonMode::LINES;
                aabbVisualisation.isDepthTestEnable = false;

                uint32 aabbPSO = api->getCache().getOrCreate(aabbVisualisation);

                aabbShader->use();
                aabbShader->setUniformMat4("view", view);
                aabbShader->setUniformMat4("projection", proj);

               // ... ваш код настройки шейдера ...

                // Итерируемся по УЗЛАМ дерева, а не по объектам
                for (const auto& node : bvh->nodes)
                {
                    // Пропускаем пустые узлы (на случай, если они есть) или
                    // рисуем все узлы подряд
                    Math::AABB3D bounds = node.bounds;

                    // Вычисляем размер и центр
                    Math::Vector3<float> size = bounds.size();
                    Math::Vector3<float> center = bounds.center();

                    auto model = Math::Mat4<float>::identity();

                    // ВАЖНО: Если ваш unitCubeMesh имеет размер от -1 до 1 (длина стороны 2),
                    // то scale на size * 0.5 корректен.
                    // Если меш от 0 до 1 (длина 1), то scale(size).
                    model = Math::scale(model, size * 0.5f);
                    model = Math::translate(model, center);

                    aabbShader->setUniformMat4("model", model);

                    // Опционально: можно менять цвет шейдера в зависимости от того,
                    // лист это (node.isLeaf()) или родительский узел.
                    if (node.isLeaf())
                    {
                        // aabbShader->setUniformVec3("color", {0, 1, 0}); // Зеленый для объектов
                    }
                    else
                    {
                        // aabbShader->setUniformVec3("color", {1, 1, 1}); // Белый для контейнеров
                    }

                    RendererCommand command{ .mesh = unitCubeMesh.get(), .pipeline = aabbPSO};
                    api->drawMesh(command);
                }
            }
        }


        //auto terrainEntities =
        //    scene.getEntitiesWith<nb::Physics::TerrainColliderComponent, TransformComponent>();

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //Ref<Shader> debugShader = rm->getResource<Shader>("aabb.shader");

        //Ref<Mesh> unitCube = rm->getResource<Mesh>("unit_cube.obj");
        //for (auto entity : terrainEntities)
        //{
        //    auto& terrComp = scene.getComponent<nb::Physics::TerrainColliderComponent>(entity.id);
        //    auto& tTerr = scene.getComponent<TransformComponent>(entity.id);
        //    auto& hm = terrComp.collider;

        //    // Если данных нет, пропускаем
        //    if (hm.heights.empty())
        //    {
        //        continue;
        //    }

        //    for (int z = 0; z < hm.rows; z += 4)
        //    {
        //        for (int x = 0; x < hm.cols; x += 4)
        //        {
        //            float height = hm.heights[z * hm.cols + x];

        //            // Пропускаем "пустые" зоны
        //            if (height < -1e9f)
        //            {
        //                continue;
        //            }

        //            float worldX = hm.minX + (float)x * hm.cellSize + tTerr.position.x;
        //            float worldZ = hm.minZ + (float)z * hm.cellSize + tTerr.position.z;
        //            float worldY = height + tTerr.position.y;

        //            nb::Math::Mat4<float> model = nb::Math::Mat4<float>::identity();
        //            model = Math::translate(model, {worldX, worldY, worldZ});

        //            float displaySize = hm.cellSize * 0.9f;
        //            model = Math::scale(model, {displaySize, 0.05f, displaySize}); // Тонкая плитка

        //            debugShader->setUniformMat4("model", model);
        //            debugShader->setUniformMat4("view", view);
        //            debugShader->setUniformMat4("projection", proj);


        //            Pipeline aabbVisualisation = {};
        //            aabbVisualisation.shader = debugShader;
        //            aabbVisualisation.polygonMode = PolygonMode::LINES;
        //            aabbVisualisation.isDepthTestEnable = true;
        //            uint32 aabbPSO = api->getCache().getOrCreate(aabbVisualisation);

        //            RendererCommand rcmd = {unitCube.get(), aabbPSO}; 
        //            api->drawMesh(rcmd);
        //        }
        //    }
        //}

        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      

        gizmoCtx.draw();

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

        RendererCommand postCmd = { .mesh = quadScreenMesh.get(), .pipeline = api->getCache().getOrCreate(quadPipeline) };
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

    void Renderer::releaseSharedContextForWindow(const SharedWindowContext& context) noexcept
    {
        api->releaseContext(context);
    }

    void Renderer::blitToWindow(const SharedWindowContext& out, const TexturePreviewRequest& request)
    {
        if (!api->setContext(out.hdc, out.hglrc)) return;

        // Используем GetClientRect вместо GetWindowRect, чтобы не учитывать рамки окна
        RECT rc;
        GetClientRect(out.handle, &rc); 
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;

        api->setViewport({ 0.0f, 0.0f, (float)width, (float)height });
        api->clear(true, false, false);
        // 1. Отрисовка сетки (Background)
        auto gridShader = ResMan::ResourceManager::getInstance()->getResource<Shader>("grid.shader");
        gridShader->use();
        
        auto mesh = contextMeshCache->get(out.hglrc, quadScreenMesh.get());
        if (!mesh) mesh = contextMeshCache->insertMesh(out.hglrc, quadScreenMesh);

        Pipeline gridPipeline = {};
        gridPipeline.shader = std::move(gridShader);
        gridPipeline.isDepthTestEnable = false;
        gridPipeline.polygonMode = PolygonMode::FULL;
        uint32 gridPso = api->getCache().getOrCreate(gridPipeline);
        
        api->drawContextMesh(*mesh, gridPso);

        // 2. Настройка смешивания для текстуры (Alpha Blending)
        // ВАЖНО: Убедитесь, что в вашем API методе установки Pipeline 
        // реализована поддержка BlendMode или включена прозрачность
        
        // 3. Отрисовка основной текстуры
        auto quadShader = ResMan::ResourceManager::getInstance()->getResource<Shader>("quadShader2.shader");
        quadShader->use();
        quadShader->setUniformInt("sceneTexture", 0);
        quadShader->setUniformVec3("channelMask", request.channelMask);
        quadShader->setUniformFloat("gamma", request.gamma);
        quadShader->setUniformFloat("exposure", request.exposure);

         
        api->bindTexture(0, request.source->getId());

        Pipeline texPipeline = {};
        texPipeline.shader = std::move(quadShader);
        texPipeline.isDepthTestEnable = false;
        texPipeline.polygonMode = PolygonMode::FULL;
        // Включаем прозрачность (в зависимости от реализации вашего API)
        texPipeline.isBlendEnable = true; // Добавьте это поле в структуру Pipeline, если его нет
        
        uint32 texPso = api->getCache().getOrCreate(texPipeline);

        api->drawContextMesh(*mesh, texPso);

        SwapBuffers(out.hdc);
        api->setDefaultContext();
    }

    void Renderer::renderMaterialPreview(
        const SharedWindowContext& out,
        MaterialPreviewRequest& request
    )
    {
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);  

        if (!api->setContext(out.hdc, out.hglrc))
        {
            return;
        }
        api->clear(true, true, false);


        auto ibl = nb::ResMan::ResourceManager::getInstance()->getResource<Resource::IhdrResource>(
            "Assets/res/glasshouse_interior_4k.hdr"
        );



        RECT rc;
        GetClientRect(out.handle, &rc);
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;
        float aspect = (float)width / (float)height;

        api->setViewport({0.0f, 0.0f, (float)width, (float)height});
        api->clear(true, true, false);


        Ref<Mesh> primitiveMesh =
            ResMan::ResourceManager::getInstance()->getResource<Mesh>("Untitled.obj");
        auto mesh = contextMeshCache->get(out.hglrc, primitiveMesh.get());
        if (!mesh)
        {
            mesh = contextMeshCache->insertMesh(out.hglrc, primitiveMesh);
        }

        Math::Vector3<float> cameraPos = {0.0f, 0.0f, -3.5f};
        Math::Vector3<float> lightPos = {0.0f, 0.0f, -3.5f};     
        Math::Vector3<float> lightColor = {15.0f, 15.0f, 15.0f}; 
        static Camera cam;
        cam.moveTo(cameraPos);
        cam.updateOrbit(request.x, request.y);

        request.x = 0.0f;
        request.y = 0.0f;

        
        Math::Mat4<float> projection = Math::projection(45.0f, aspect, 0.1f, 100.0f);
        Math::Mat4<float> view = cam.getLookAt();
        Math::Mat4<float> model = Math::Mat4<float>::identity();

        static Skybox sky;
        auto skyboxShader =
            nb::ResMan::ResourceManager::getInstance()->getResource<nb::Renderer::Shader>(
                "skybox.shader"
            );



        sky.bindCubemap(ibl->getCubemap());
        skyboxShader->setUniformInt("skybox", 0);
        skyboxShader->setUniformMat4("view", view);
        skyboxShader->setUniformMat4("projection", projection);
        sky.render(skyboxShader);

        for (int i = 0; i < 7; i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0); // Привязываем 0 к 2D таргету
        }

        for (int i = 0; i < 7; i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0); // Привязываем 0 к 2D таргету
        }


        auto shader = request.material->getShader();
        request.material->bind(api);
        //shader->use(); // Явно биндим шейдер перед установкой юниформов

        // --- 1. ТЕКСТУРЫ МАТЕРИАЛА ---
        // Слот 0: Albedo
        auto albedoTex = std::get<Ref<nb::Resource::TextureAsset>>(
            request.material->getProperties()["u_AlbedoMap"].value
        );
        //shader->setUniformInt("u_AlbedoMap", 0);
        api->bindTexture(0, albedoTex->getInternalTexture()->getId());

        // Слот 1: Normal
        auto normalTex = std::get<Ref<nb::Resource::TextureAsset>>(
            request.material->getProperties()["u_NormalMap"].value
        );
        //shader->setUniformInt("u_NormalMap", 1);
        api->bindTexture(1, normalTex->getInternalTexture()->getId());

        // Слот 2: ORM (Убедись, что это не HDR панорама!)
        auto ormTex = std::get<Ref<nb::Resource::TextureAsset>>(
            request.material->getProperties()["u_ORMMap"].value
        );
        //shader->setUniformInt("u_ORMMap", 2);

        api->bindTexture(2, ormTex->getInternalTexture()->getId());

        // --- 2. СИСТЕМНЫЕ ТЕКСТУРЫ IBL ---
        




        // Слот 3: Irradiance (CUBE)
        //shader->setUniformInt("u_IrradianceMap", 3);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->getIrradianceCubemap()->getId());

        // Слот 4: Prefilter (CUBE)
        //shader->setUniformInt("u_PrefilterMap", 4);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->getPrefilterCubemap()->getId());

        // Слот 5: BRDF LUT (2D)
        //shader->setUniformInt("u_BrdfLUT", 5);
        //api->bindTexture(5, ibl->getBrdfTexture()->getId());
        api->bindTexture(0, albedoTex->getInternalTexture()->getId());
        api->bindTexture(1, normalTex->getInternalTexture()->getId());
        api->bindTexture(2, ormTex->getInternalTexture()->getId());
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->getIrradianceCubemap()->getId());

        // Слот 4: Prefilter (CUBE)
        // shader->setUniformInt("u_PrefilterMap", 4);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->getPrefilterCubemap()->getId());

        // Слот 5: BRDF LUT (2D)
        // shader->setUniformInt("u_BrdfLUT", 5);
        api->bindTexture(5, ibl->getBrdfTexture()->getId());




        



        shader->setUniformMat4("u_Projection", projection);
        shader->setUniformMat4("u_View", view);
        shader->setUniformMat4("u_Model", model);





        shader->setUniformVec3("u_CameraPos", cam.getPosition());
        shader->setUniformVec3("u_LightPos", lightPos);
        shader->setUniformVec3("u_LightColor", lightColor);
        
        Pipeline matPipeline = {};
        matPipeline.shader = shader;
        matPipeline.isDepthTestEnable = true;
        matPipeline.isBlendEnable = true;
        matPipeline.polygonMode = PolygonMode::FULL;

        uint32 matPso = api->getCache().getOrCreate(matPipeline);
        api->drawContextMesh(*mesh, matPso);

        SwapBuffers(out.hdc);
        api->setDefaultContext();
    }

    tinygizmo::gizmo_context& Renderer::getGizmoContext() noexcept
    {
        return gizmoCtx;
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
            RendererCommand debugPassCommand{ .mesh = gizmoMesh.get(), .pipeline = pso };
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


        Ref<Mesh> cube = rm->getResource<Mesh>("sphere_exp.obj");

        nb::Node cubeNode = scene.createNode();
        cubeNode.setName("cube");

        auto& cubeTransform = cubeNode.getComponent<TransformComponent>();
        cubeTransform.position = {0.0f, 200.0f, 0.0f};
        cubeTransform.scale = {1.0f, 1.0f, 1.0f};
        cubeTransform.dirty = true;
        cube->uniforms.shader = shader;
        auto material = rm->getResource<nb::Resource::MaterialAsset>("Assets/res/plastic.material");
        auto brickMaterial = rm->getResource<nb::Resource::MaterialAsset>("Assets/res/brick.material");

        cubeNode.addComponent(MeshComponent{cube, brickMaterial});
        auto aabb = cube->getAabb3d();
        
        cubeNode.addComponent(Physics::Collider { .halfSize = (aabb.halfSize() * 1.0f)  });

        cubeNode.addComponent(
            Physics::Rigidbody{
                .velocity = {0.0f, 0.0f, 0.0f},
                .acceleration = {0.0f, 0.0f, 0.0f},
                .mass = 1.0f,
                .useGravity = true
            }
        );
        cubeNode.addComponent(
            nb::Script::ScriptComponent{
                .script = std::make_shared<nb::Script::Script>(
                    nb::Script::ScriptEngineSingleton::instance(), "aa.lua"
                )
            }
        );


        Ref<Mesh> surf = rm->getResource<Mesh>("reddd.obj");

        nb::Node surfNode = scene.createNode();
        surfNode.setName("scene");

        auto& surfTransform = surfNode.getComponent<TransformComponent>();
        surfTransform.position = {0.0f, 0.0f, 0.0f};
        surfTransform.scale = {0.1f, 0.1f, 0.1f};
        surfTransform.dirty = true;
        surf->uniforms.shader = shader;

        surfNode.addComponent(MeshComponent{surf, material});
        //surfNode.addComponent(Physics::Collider{.halfSize = {100.0f, 1.0f, 100.0f}});
        surfNode.addComponent(Physics::GroundTag());
        surfNode.addComponent(Physics::TerrainColliderComponent());
        auto bakedData = nb::Physics::bakeMesh(*surf, 0.1f);
        surfNode.getComponent<Physics::TerrainColliderComponent>().collider = std::move(bakedData);

        //surfNode.addComponent(nb::Script::ScriptComponent())

        //nb::Serialize::IArchive* archive =
        //    new nb::Serialize::JsonArchive("Assets/res/Scene.json");
        //Scene::getInstance().serialize(archive);
        //delete archive;
        //

       /* nb::Serialize::IArchive* archive =
            new nb::Serialize::JsonArchive("Assets/res/Scene.json");
        archive->setMode(nb::Serialize::JsonArchive::Mode::READ);
        archive->load();
        Scene::getInstance().deserialize(archive);
        delete archive;*/
    }
}
