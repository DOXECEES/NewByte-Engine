// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Renderer.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../../dependencies/stb/stb_image_write.h"
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

#include "Math/RayCast/RayPicker.hpp"

#include "OpenGL/Placeholder.hpp"

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

        skybox = std::make_unique<Skybox>();
        
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
        mainFrameBuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::COLOR_HDR);
        mainFrameBuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::COLOR_HDR);
        mainFrameBuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::COLOR_HDR);
        mainFrameBuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::COLOR_HDR);
        mainFrameBuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::DEPTH);
        mainFrameBuffer->finalize();

        mainFrameBuffer->setDrawBuffers(4);

        shadowFrameBuffer = api->createFrameBuffer(2048, 2048);
        shadowFrameBuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::DEPTH);
        shadowFrameBuffer->finalize();
        
        ssrResultBuffer = api->createFrameBuffer(width, heigth);
        ssrResultBuffer->addTextureAttachment(IFrameBuffer::TextureAttachment::COLOR);
        ssrResultBuffer->finalize();

        pointShadowFrameBuffer = api->createFrameBuffer(1024, 1024);
        pointShadowFrameBuffer->addRenderBufferAttachment(
            IFrameBuffer::RenderBufferAttachment::DEPTH
        );

        pointShadowFrameBuffer->bind();
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        
       pointShadowFrameBuffer->setDrawBuffers(1);
        pointShadowFrameBuffer->finalize();

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

    namespace
    {
        constexpr float SHADOW_MAP_RESOLUTION = 2048.0f;
        constexpr float ORTHO_SIZE            = 45.0f;
        constexpr float LIGHT_DISTANCE        = 30.0f;
        constexpr float Z_NEAR                = 0.1f;
        constexpr float Z_FAR                 = 75.0f;
        constexpr float CLEAR_ALPHA           = 1.0f;
        constexpr float POINT_SHADOW_RES      = 1024.0f;

        const std::string_view PLACEHOLDER_MATERIAL_PATH = "Assets/materials/placeholder.material";
        const std::string_view DEFAULT_IBL_PATH          = "Assets/res/grasslands_sunset_4k.hdr";
        const std::string_view SHADOW_SHADER_NAME        = "lightPass.shader";
        const std::string_view MAIN_SHADER_NAME          = "ADS.shader";
    }


    void Renderer::render() noexcept
    {
        const int width  = nb::Core::EngineSettings::getWidth();
        const int height = nb::Core::EngineSettings::getHeight();

        if (width <= 0 || height <= 0)
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::WARNING, "Invalid viewport dimensions")
                .with("width", width)
                .with("height", height);
            return;
        }

        auto* resourceManager = ResMan::ResourceManager::getInstance();
        auto& scene           = nb::Scene::getInstance();
        auto& registry        = scene.getRegistry();

        onResize(width, height);

        nbstl::Vector<RendererCommand> mainQueue;
        std::vector<Ecs::EntityID>     directionalLights;
        std::vector<Ecs::EntityID>     pointLights;

        directionalLights.reserve(8);
        pointLights.reserve(32);

        auto mainShader = resourceManager->getResource<Shader>(MAIN_SHADER_NAME.data());

        scene.traverseAll(
            [&](Ecs::EntityID entityId)
            {
                Ecs::Entity entity{entityId};

                if (registry.has<LightComponent>(entity))
                {
                    const auto& light = registry.get<LightComponent>(entity);
                    if (light.isPointLight())
                    {
                        pointLights.push_back(entityId);
                    }
                    else
                    {
                        directionalLights.push_back(entityId);
                    }
                }

                if (registry.has<MeshComponent>(entity) && registry.has<TransformComponent>(entity))
                {
                    auto& meshComp  = registry.get<MeshComponent>(entity);
                    auto& transform = registry.get<TransformComponent>(entity);

                    if (meshComp.material.empty())
                    {
                        auto placeholder = resourceManager->getResource<Resource::MaterialAsset>(
                            PLACEHOLDER_MATERIAL_PATH.data()
                        );
                        meshComp.material.push_back(placeholder);
                    }

                    Pipeline pipelineConfig{};
                    pipelineConfig.shader      = mainShader;
                    pipelineConfig.polygonMode = polygonMode;

                    mainQueue.pushBack(
                        {.mesh     = meshComp.mesh.get(),
                         .material = meshComp.material,
                         .pipeline = api->getCache().getOrCreate(pipelineConfig),
                         .model    = transform.worldMatrix}
                    );
                }
            }
        );

        api->beginFrame();

        if (!isPreviewInitialized)
        {
            saveSpherePreview("Assets/res/brick.material", "Assets/materials/material.png");
            isPreviewInitialized = true;
        }


        Math::Mat4<float> currentLightView = Math::Mat4<float>::identity();
        Math::Mat4<float> currentLightProj = Math::Mat4<float>::identity();
        
        if (!directionalLights.empty())
        {
            api->setViewport({0, 0, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION});
            api->bindFrameBuffer(shadowFrameBuffer);
            api->setClearColor(Colors::BROWN, CLEAR_ALPHA, 0);
            api->clear(false, true, false);

            const auto& mainLight = registry.get<LightComponent>(Ecs::Entity{directionalLights[0]});
            nb::Math::Vector3<float> lightDir = mainLight.direction;
            lightDir.normalize();

            const nb::Math::Vector3<float> sceneCenter{0.0f, 0.0f, 0.0f};
            const nb::Math::Vector3<float> lightPos = sceneCenter - (lightDir * LIGHT_DISTANCE);
            const nb::Math::Vector3<float> upVector{0.0f, 1.0f, 0.0f};

            currentLightProj =
                nb::Math::ortho(-ORTHO_SIZE, ORTHO_SIZE, -ORTHO_SIZE, ORTHO_SIZE, Z_NEAR, Z_FAR);
            currentLightView = nb::Math::lookAt(lightPos, sceneCenter, upVector);

            auto     shadowShader = resourceManager->getResource<Shader>(SHADOW_SHADER_NAME.data());
            Pipeline shadowPipeline{.shader = shadowShader, .polygonMode = PolygonMode::FULL};
            uint32   shadowPso = api->getCache().getOrCreate(shadowPipeline);

            shadowShader->use();
            shadowShader->setUniformMat4("lightProj", currentLightProj);
            shadowShader->setUniformMat4("lightView", currentLightView);

            for (const auto& cmd : mainQueue)
            {
                shadowShader->setUniformMat4("model", cmd.model);
                RendererCommand shadowCmd{.mesh = cmd.mesh, .pipeline = shadowPso};
                api->drawMesh(shadowCmd);
            }
        }

        if (!pointLights.empty())
        {
            const float POINT_FAR_PLANE = 50.0f;
            auto        pointShadowShader =
                resourceManager->getResource<Shader>("point_shadow_gen.shader");
            Pipeline pointShadowPipeline{
                .shader = pointShadowShader, .polygonMode = PolygonMode::FULL
            };
            uint32 pointShadowPso = api->getCache().getOrCreate(pointShadowPipeline);

            api->setViewport({0, 0, POINT_SHADOW_RES, POINT_SHADOW_RES});

            pointShadowShader->use();

            for (auto id : pointLights)
            {
                const auto& light     = registry.get<LightComponent>(Ecs::Entity{id});
                const auto& transform = registry.get<TransformComponent>(Ecs::Entity{id});

                if (!light.castShadows)
                {
                    continue;
                }

                auto it = m_pointShadowMaps.find(id);
                if (it == m_pointShadowMaps.end())
                {
                    // Исправленное создание кубмапы (без GL_NEAREST)
                    CubemapParameters params;

                    params.size = POINT_SHADOW_RES;
                    params.format = CubemapParameters::Format::R32Float;
                    params.generateMipmaps = false;

                    params.wrapU = CubemapParameters::Wrapping::ClampToEdge;
                    params.wrapV = CubemapParameters::Wrapping::ClampToEdge;
                    params.wrapW = CubemapParameters::Wrapping::ClampToEdge;

                    params.minFilter = CubemapParameters::Filtering::Linear;
                    params.magFilter = CubemapParameters::Filtering::Linear;

                    auto newCubemap = api->createCubemap(params);
                    

                    it = m_pointShadowMaps.insert({id, newCubemap}).first;
                }

                Ref<Cubemap>  shadowMap = it->second;
                Math::Vector3 pos       = transform.position;

                pointShadowShader->setUniformVec3("u_LightPos", pos);
                pointShadowShader->setUniformFloat("u_FarPlane", POINT_FAR_PLANE);

                Math::Mat4<float> shadowProj =
                    Math::projection(Math::toRadians(90.0f), 1.0f, 0.1f, POINT_FAR_PLANE);

                Math::Mat4<float> shadowViews[6] = {
                    Math::lookAt(pos, pos + Math::Vector3<float>{1, 0, 0}, {0, -1, 0}),
                    Math::lookAt(pos, pos + Math::Vector3<float>{-1, 0, 0}, {0, -1, 0}),
                    Math::lookAt(pos, pos + Math::Vector3<float>{0, 1, 0}, {0, 0, 1}),
                    Math::lookAt(pos, pos + Math::Vector3<float>{0, -1, 0}, {0, 0, -1}),
                    Math::lookAt(pos, pos + Math::Vector3<float>{0, 0, 1}, {0, -1, 0}),
                    Math::lookAt(pos, pos + Math::Vector3<float>{0, 0, -1}, {0, -1, 0})
                };

                for (int i = 0; i < 6; ++i)
                {
                    // === НОВЫЙ ПОДХОД К ПРИВЯЗКЕ ===
                    // 1. Биндим FBO (у него только depth renderbuffer)
                    pointShadowFrameBuffer->bind();

                    glFramebufferTexture2D(
                        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                        shadowMap->getId(), 0
                    );

                    // ВАЖНО: Очищаем белым (1.0 = максимальная дальность)
                    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
                    api->clear(true, true, false);


                    // 5. Шейдер и рендер
                    pointShadowShader->setUniformMat4("u_View", shadowViews[i]);
                    pointShadowShader->setUniformMat4("u_Projection", shadowProj);

                    for (const auto& cmd : mainQueue)
                    {
                        pointShadowShader->setUniformMat4("model", cmd.model);
                        api->drawMesh({.mesh = cmd.mesh, .pipeline = pointShadowPso});
                    }
                    // ==================================
                }
                // После рендера всех 6 граней для этого источника
                // Убедимся, что FBO не хранит лишнего состояния
                glFramebufferTexture2D(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0
                );
            }

            pointShadowFrameBuffer->unBind();
            api->setClearColor(nb::Colors::BLACK, 1.0f, 0);
        }
        glMemoryBarrier(
            GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT |
            GL_FRAMEBUFFER_BARRIER_BIT
        );


        api->bindDefaultFrameBuffer();
        api->bindFrameBuffer(mainFrameBuffer);
        api->setViewport({0, 0, static_cast<float>(width), static_cast<float>(height)});
        api->setClearColor(Colors::BLACK, CLEAR_ALPHA, 0);
        api->clear(true, true, false);

        const auto view   = cam->getLookAt();
        const auto proj   = cam->getProjection();
        const auto camPos = cam->getPosition();

        auto iblResource =
            resourceManager->getResource<Resource::IhdrResource>(DEFAULT_IBL_PATH.data());
        if (iblResource)
        {
            auto skyboxShader = resourceManager->getResource<nb::Renderer::Shader>("skybox.shader");
            skyboxShader->use();
            skyboxShader->setUniformInt("skybox", 0);
            skyboxShader->setUniformMat4("view", view);
            skyboxShader->setUniformMat4("projection", proj);

            skybox->bindCubemap(iblResource->getCubemap());
            skybox->render(skyboxShader);
        }

        if (isShowGridEnabled)
        {
            auto gridShader =
                resourceManager->getResource<nb::Renderer::Shader>("infinite_grid.shader");
            Pipeline gridPipeline{
                .shader = gridShader, .isDepthTestEnable = false, .isBlendEnable = true
            };

            gridShader->use();
            gridShader->setUniformVec3("uCameraWorldPosition", camPos);
            gridShader->setUniformMat4("uViewProjection", view * proj);

            RendererCommand gridCmd{
                .mesh        = nullptr,
                .pipeline    = api->getCache().getOrCreate(gridPipeline),
                .vertexCount = 6
            };
            api->drawVertexless(gridCmd);
        }

        std::vector<PointLight>       pointLightsData;
        std::vector<DirectionalLight> dirLightsData;

        for (auto id : directionalLights)
        {
            const auto& l = registry.get<LightComponent>(Ecs::Entity{id});
            dirLightsData.emplace_back(
                l.ambient.asVec3(), l.diffuse.asVec3(), l.specular.asVec3(), l.direction
            );
        }
        for (auto id : pointLights)
        {
            const auto& l = registry.get<LightComponent>(Ecs::Entity{id});
            const auto& t = registry.get<TransformComponent>(Ecs::Entity{id});
            auto&       data = pointLightsData.emplace_back(
                l.ambient.asVec3(), l.diffuse.asVec3(), l.specular.asVec3(), t.position, l.constant,
                l.linear, l.quadratic, 1.0f
            );

            if (l.castShadows && m_pointShadowMaps.contains(id))
            {
                data.shadowMapHandle = m_pointShadowMaps[id]->getHandle();
                data.farPlane        = 50.0f;
                data.hasShadow       = true;
            }
            else
            {
                data.hasShadow = false;
            }

        }

        //api->bindTexture(3, shadowFrameBuffer->getTexture());
        if (iblResource)
        {
            api->bindCubemap(4, iblResource->getIrradianceCubemap()->getId());
            api->bindCubemap(5, iblResource->getPrefilterCubemap()->getId());
            api->bindTexture(6, iblResource->getBrdfTexture()->getId());
        }
        

        


        for (auto& cmd : mainQueue)
        {
            auto shader = cmd.material[0]->getShader();
            shader->use();

            for (uint32_t i = 0; i < 2; ++i)
            {
                // 1. Получаем ID сущности (предположим, у вас есть доступ к массиву pointLights)
                auto id = pointLights[i];
                auto it = m_pointShadowMaps.find(id);

                if (it == m_pointShadowMaps.end())
                {
                    nb::Error::ErrorManager::instance()
                        .report(nb::Error::Type::WARNING, "Shadow Map missing in Cache")
                        .with("Light Index", (int)i);
                    continue;
                }

                // 2. Проверка самого объекта Cubemap
                auto shadowMap = it->second;
                if (!shadowMap)
                {
                    nb::Error::ErrorManager::instance()
                        .report(nb::Error::Type::FATAL, "Cubemap object is null")
                        .with("Index", (int)i);
                    continue;
                }

                // 3. Проверка хендла
                uint64_t handle = shadowMap->getHandle();
                if (handle == 0)
                {
                    nb::Error::ErrorManager::instance()
                        .report(nb::Error::Type::FATAL, "Bindless handle is ZERO")
                        .with("Texture ID", (int)shadowMap->getId())
                        .with("Hint", "Check if finalizeBindless() was called");
                }

                // 4. Проверка резидентности (должно быть true)
                if (handle != 0 && !glIsTextureHandleResidentARB(handle))
                {
                    nb::Error::ErrorManager::instance()
                        .report(nb::Error::Type::FATAL, "Handle is NOT resident")
                        .with("Handle Value", (long long)handle);
                }

                // 5. Проверка локации в шейдере
                std::string shadowHandleName =
                    "lightPoint[" + std::to_string(i) + "].shadowMapHandle";
                GLint loc = glGetUniformLocation(shader->getId(), shadowHandleName.c_str());

                if (loc == -1)
                {
                    nb::Error::ErrorManager::instance()
                        .report(
                            nb::Error::Type::WARNING, "Uniform location not found (Optimized out?)"
                        )
                        .with("Uniform Name", shadowHandleName);
                }
                else
                {
                    // Если всё ок, выводим инфо о успешной привязке
                    nb::Error::ErrorManager::instance()
                        .report(nb::Error::Type::INFO, "Bindless shadow handle status")
                        .with("Index", (int)i)
                        .with("Loc", loc)
                        .with("Handle", (long long)handle);
                }
            }

            shader->setUniformUint64("shadowMap", shadowFrameBuffer->getTextureHandle(0));
            shader->setUniformVec3("u_CameraPos", camPos);
            shader->setUniformMat4("model", cmd.model);
            shader->setUniformMat4("view", view);
            shader->setUniformMat4("proj", proj);
            shader->setUniformMat4("lightView", currentLightView);
            shader->setUniformMat4("lightProj", currentLightProj);


            shader->setUniformUint64("u_EmissionMap", OpenGl::createPlaceholderForEmission());

            for (auto& l : dirLightsData)
            {
                l.applyUniforms(shader);
            }
            for (auto& l : pointLightsData)
            {
                l.applyUniforms(shader);
            }

            shader->setUniformInt(
                ShaderConstants::COUNT_OF_DIRECTIONLIGHT_UNIFORM_NAME.data(),
                static_cast<int>(dirLightsData.size())
            );
            shader->setUniformInt(
                ShaderConstants::COUNT_OF_POINTLIGHT_UNIFORM_NAME.data(),
                static_cast<int>(pointLightsData.size())
            );

            api->drawMesh(cmd);
        }

        renderDebugPasses(view, proj, directionalLights, pointLights, mainQueue);

        renderSSR(width, height, view, proj);

        renderFinalQuad(width, height);

        api->endFrame();
    }

    void Renderer::renderDebugPasses(
        const nb::Math::Mat4<float>&          view,
        const nb::Math::Mat4<float>&          proj,
        const std::vector<Ecs::EntityID>&     dirLights,
        const std::vector<Ecs::EntityID>&     pointLights,
        const nbstl::Vector<RendererCommand>& mainQueue
    ) noexcept
    {
        auto* rm       = ResMan::ResourceManager::getInstance();
        auto& registry = nb::Scene::getInstance().getRegistry();

        if (isDebugPassEnabled)
        {
            Pipeline debugP = {
                .shader            = debugLightShader,
                .polygonMode       = PolygonMode::LINES,
                .isDepthTestEnable = true
            };
            uint32 debugPso = api->getCache().getOrCreate(debugP);
            debugLightShader->use();
            debugLightShader->setUniformMat4("view", view);
            debugLightShader->setUniformMat4("proj", proj);

            auto drawLightGizmo = [&](Ecs::EntityID id)
            {
                const auto&    trans = registry.get<TransformComponent>(Ecs::Entity{id});
                const auto&    light = registry.get<LightComponent>(Ecs::Entity{id});
                nb::Math::Mat4 model =
                    nb::Math::translate(nb::Math::Mat4<float>::identity(), trans.position);
                debugLightShader->setUniformMat4("model", model);
                debugLightShader->setUniformVec3("u_Color", light.ambient.asVec3());
                api->drawMesh(RendererCommand{.mesh = debugLightMesh.get(), .pipeline = debugPso});
            };

            for (auto id : dirLights)
            {
                drawLightGizmo(id);
            }
            for (auto id : pointLights)
            {
                drawLightGizmo(id);
            }
        }

        if (isBoundingBoxVisualizationEnabled || isBVHVisualizationEnabled)
        {
            auto     aabbShader = rm->getResource<Shader>("aabb.shader");
            auto     unitCube   = rm->getResource<Mesh>("unit_cube.obj");
            Pipeline aabbP      = {
                .shader = aabbShader, .polygonMode = PolygonMode::LINES, .isDepthTestEnable = false
            };
            uint32 aabbPso = api->getCache().getOrCreate(aabbP);

            aabbShader->use();
            aabbShader->setUniformMat4("view", view);
            aabbShader->setUniformMat4("projection", proj);

            if (isBoundingBoxVisualizationEnabled)
            {
                for (const auto& cmd : mainQueue)
                {
                    nb::Math::AABB3D worldAabb = nb::Math::AABB3D::recalculateAabb3dByModelMatrix(
                        cmd.mesh->getAabb3d(), cmd.model
                    );
                    nb::Math::Mat4 model = nb::Math::Mat4<float>::identity();
                    model                = nb::Math::scale(model, worldAabb.size() * 0.5f);
                    model                = nb::Math::translate(model, worldAabb.center());
                    aabbShader->setUniformMat4("model", model);
                    api->drawMesh({.mesh = unitCube.get(), .pipeline = aabbPso});
                }
            }

            if (isBVHVisualizationEnabled)
            {
                auto bvh = nb::Scene::getInstance().getBvh();
                for (const auto& node : bvh->nodes)
                {
                    nb::Math::Mat4 model = nb::Math::Mat4<float>::identity();
                    model                = nb::Math::scale(model, node.bounds.size() * 0.5f);
                    model                = nb::Math::translate(model, node.bounds.center());
                    aabbShader->setUniformMat4("model", model);
                    api->drawMesh({.mesh = unitCube.get(), .pipeline = aabbPso});
                }
            }
        }

        gizmoCtx.draw();
    }

    void Renderer::renderSSR(
        int                   width,
        int                   height,
        const nb::Math::Mat4<float>& view,
        const nb::Math::Mat4<float>& proj
    ) noexcept
    {
        auto ssrShader = ResMan::ResourceManager::getInstance()->getResource<Shader>("ssr.shader");

        api->bindFrameBuffer(ssrResultBuffer);
        api->setViewport({0, 0, static_cast<float>(width), static_cast<float>(height)});
        api->clear(true, false, false);

        ssrShader->use();
        for (uint32 i = 0; i < 5; ++i)
        {
            api->bindTexture(i, mainFrameBuffer->getTexture(i));
        }

        ssrShader->setUniformMat4("invView", nb::Math::inverseWithoutTranspose(view));
        ssrShader->setUniformMat4("invProjection", nb::Math::inverseWithoutTranspose(proj));
        ssrShader->setUniformMat4("projection", proj);
        ssrShader->setUniformMat4("view", view);
        ssrShader->setUniformVec2(
            "u_ScreenSize", {static_cast<float>(width), static_cast<float>(height)}
        );

        Pipeline ssrP = {.shader = ssrShader, .isDepthTestEnable = false};
        api->drawMesh(
            {.mesh = quadScreenMesh.get(), .pipeline = api->getCache().getOrCreate(ssrP)}
        );
    }

    void Renderer::renderFinalQuad(
        int width,
        int height
    ) noexcept
    {
        auto quadShader = ResMan::ResourceManager::getInstance()->getResource<Shader>(
            "quadShader.shader", {"USE_FXAA"}
        );

        api->bindDefaultFrameBuffer();
        api->setViewport({0, 0, static_cast<float>(width), static_cast<float>(height)});
        api->setClearColor(Colors::WHITE, CLEAR_ALPHA, 0);
        api->clear(true, false, false);

        quadShader->use();
        quadShader->setUniformInt("depthMap", 3);
        quadShader->setUniformVec2(
            "screenSize", {static_cast<float>(width), static_cast<float>(height)}
        );

        api->bindTexture(3, ssrResultBuffer->getTexture(0));

        Pipeline quadP = {
            .shader = quadShader, .polygonMode = PolygonMode::FULL, .isDepthTestEnable = false
        };
        api->drawMesh(
            {.mesh = quadScreenMesh.get(), .pipeline = api->getCache().getOrCreate(quadP)}
        );
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

    void Renderer::pickNodeAndApplyMaterial(
        int                          x,
        int                          y,
        const std::filesystem::path& path
    ) noexcept
    {
        Math::RayPicker picker;
        Math::Ray ray = picker.cast(
            cam, x, y, Core::EngineSettings::getWidth(), Core::EngineSettings::getHeight()
        );

        auto&         scene = Scene::getInstance();
        Ecs::EntityID id = scene.pickNode(ray);
        Node node = scene.getNode(id);

        if (node.isValid() && node.hasComponent<MeshComponent>())
        {
            auto& meshComponent = node.getComponent<MeshComponent>();
            
            Ref<Resource::MaterialAsset> asset =
                ResMan::ResourceManager::getInstance()->getResource<Resource::MaterialAsset>(path.string());

            meshComponent.material = {asset};
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

         
        api->bindTexture(0, request.source);

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

    void Renderer::renderShadowPreview(
        const SharedWindowContext& out,
        uint32_t                   shadowTextureId,
        float                      nearPlane,
        float                      farPlane
    )
    {
        if (!api->setContext(out.hdc, out.hglrc))
        {
            return;
        }

        RECT rc;
        GetClientRect(out.handle, &rc);
        int width  = rc.right - rc.left;
        int height = rc.bottom - rc.top;

        api->setViewport({0.0f, 0.0f, (float)width, (float)height});
        api->clear(true, false, false); // Очистка цветом

        // 2. Получение меша (Full Screen Quad)
        auto mesh = contextMeshCache->get(out.hglrc, quadScreenMesh.get());
        if (!mesh)
        {
            mesh = contextMeshCache->insertMesh(out.hglrc, quadScreenMesh);
        }

        // 3. Работа с шейдером визуализации теней
        // Рекомендуется использовать специальный шейдер, который умеет делать линеаризацию
        auto shadowVizShader =
            ResMan::ResourceManager::getInstance()->getResource<Shader>("shadow_viz.shader");
        shadowVizShader->use();

        // Передаем параметры для корректного отображения глубины
        shadowVizShader->setUniformInt("shadowMap", 0);
        //shadowVizShader->setUniformFloat("near_plane", nearPlane); // например, 0.1f
        //shadowVizShader->setUniformFloat("far_plane", farPlane);   // например, 100.0f
        //shadowVizShader->setUniformInt()
        // Биндим текстуру тени
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadowTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);



        // 4. Настройка конвейера (Pipeline)
        Pipeline pipeline          = {};
        pipeline.shader            = std::move(shadowVizShader);
        pipeline.isDepthTestEnable = false; // Нам не нужен тест глубины для вывода на экран
        pipeline.isBlendEnable     = false; // Выводим как есть
        pipeline.polygonMode       = PolygonMode::FULL;

        uint32 pso = api->getCache().getOrCreate(pipeline);

        // 5. Отрисовка
        api->drawContextMesh(*mesh, pso);

        // 6. Вывод на экран
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

        //for (int i = 0; i < 7; i++)
        //{
        //    glActiveTexture(GL_TEXTURE0 + i);
        //    glBindTexture(GL_TEXTURE_2D, 0); // Привязываем 0 к 2D таргету
        //}

        //for (int i = 0; i < 7; i++)
        //{
        //    glActiveTexture(GL_TEXTURE0 + i);
        //    glBindTexture(GL_TEXTURE_CUBE_MAP, 0); // Привязываем 0 к 2D таргету
        //}


        auto shader = request.material->getShader();
        request.material->bind();
        


        // Слот 3: Irradiance (CUBE)
        //shader->setUniformInt("u_IrradianceMap", 3);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->getIrradianceCubemap()->getId());

        // Слот 4: Prefilter (CUBE)
        //shader->setUniformInt("u_PrefilterMap", 4);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->getPrefilterCubemap()->getId());

        // Слот 5: BRDF LUT (2D)
        //shader->setUniformInt("u_BrdfLUT", 5);
        api->bindTexture(6, ibl->getBrdfTexture()->getId());
        




        



        shader->setUniformMat4("proj", projection);
        shader->setUniformMat4("view", view);
        shader->setUniformMat4("model", model);





        shader->setUniformVec3("u_CameraPos", cam.getPosition());
        //shader->setUniformVec3("u_LightPos", lightPos);
        //shader->setUniformVec3("u_LightColor", lightColor);
        
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



    void Renderer::saveSpherePreview(
        const std::filesystem::path& materialPath,
        const std::string&           savePath
    )
    {
        const int size = 512;
        auto      rm   = ResMan::ResourceManager::getInstance();

        auto tempFB = api->createFrameBuffer(size, size);
        tempFB->addTextureAttachment(IFrameBuffer::TextureAttachment::COLOR);
        tempFB->addRenderBufferAttachment(IFrameBuffer::RenderBufferAttachment::DEPTH_STENCIL);
        tempFB->finalize();

        // Сфера радиусом 1
        Ref<Mesh> sphereMesh    = rm->getResource<Mesh>("unit_cube.obj");
        auto      materialAsset = rm->getResource<Resource::MaterialAsset>(materialPath.string());

        // МАТРИЦЫ: Отодвигаем камеру дальше (Z = 4.0)
        Math::Mat4<float>    projection = Math::projection(90.0f, 1.0f, 0.1f, 100.0f);
        Math::Vector3<float> camPoss     = {0.0f, 1.0f, 1.8f}; // Чуть выше и дальше
        Math::Mat4<float>    view       = Math::lookAt(
            camPoss, Math::Vector3<float>{0.0f, 0.0f, 0.0f}, Math::Vector3<float>{0.0f, 1.0f, 0.0f}
        );
        Math::Mat4<float> model = Math::Mat4<float>::identity();

        model = Math::translate(model, {0.0f, 0.0f, 0.0f});

        api->bindFrameBuffer(tempFB);
        api->setViewport({0, 0, (float)size, (float)size});
        api->setClearColor(Colors::GRAY, 1.0f, 0);
        api->clear(true, true, false);

        // Подключаем HDR карту (IBL)
        auto ibl = rm->getResource<Resource::IhdrResource>("Assets/res/grasslands_sunset_4k.hdr");

        auto shader = materialAsset->getShader();
        shader->use();

        // Передаем Uniform-переменные
        shader->setUniformMat4("proj", projection);
        shader->setUniformMat4("view", view);
        shader->setUniformMat4("model", model);
        shader->setUniformVec3("u_CameraPos", camPoss); // ОБЯЗАТЕЛЬНО для бликов PBR

        // Биндим текстуры материала
        materialAsset->bind();

        // Биндим карты IBL (как в вашем основном методе render)
        if (ibl)
        {
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->getIrradianceCubemap()->getId());
            shader->setUniformInt("u_IrradianceMap", 4); // Проверьте имя в шейдере

            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->getPrefilterCubemap()->getId());
            shader->setUniformInt("u_PrefilterMap", 5);

            api->bindTexture(6, ibl->getBrdfTexture()->getId());
            shader->setUniformInt("u_BrdfLUT", 6);
        }

        // Рендерим сферу
        Pipeline matPipeline = {
            .shader = shader, .isDepthTestEnable = true
        };
        uint32          pso = api->getCache().getOrCreate(matPipeline);
        RendererCommand cmd = {.mesh = sphereMesh.get(), .pipeline = pso};
        api->drawMesh(cmd);

        // Читаем пиксели
        std::vector<unsigned char> data(size * size * 4);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, size, size, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

        stbi_flip_vertically_on_write(true);
        stbi_write_png(savePath.c_str(), size, size, 4, data.data(), size * 4);

        api->bindDefaultFrameBuffer();
    }

    void Renderer::loadSceneEcs() noexcept
    {
        //auto rm = ResMan::ResourceManager::getInstance();
        //auto shader = rm->getResource<Shader>("ADS.shader");

        //auto& scene = nb::Scene::getInstance();

        //Math::Vector3<float> ambientColor{0.0f, 0.0f, 0.0f};


        //nb::Node dirLight = scene.createNode();
        //dirLight.setName("DirLight");

        //dirLight.addComponent(
        //    LightComponent{
        //        LightType::DIRECTIONAL,
        //        Colors::BLACK,
        //        Colors::WHITE,
        //        Color::fromLinearRgb(0.1f, 0.1f, 0.1f),
        //        {-1.0f, -0.2f, -0.2f}
        //    }
        //);

        //nb::Node pointLight = scene.createNode(dirLight.getId());
        //pointLight.setName("PointLight");

        //auto& plTransform = pointLight.getComponent<TransformComponent>();
        //plTransform.position = {-5.0f, 19.0f, -5.0f};
        //plTransform.dirty = true;

        //pointLight.addComponent(
        //    LightComponent{
        //        LightType::POINT,
        //        Colors::BLACK,
        //        Colors::WHITE,
        //        Color::fromLinearRgb(0.1f, 0.1f, 0.1f),
        //        {0.0f,0.0f,0.0f},
        //        1.0f,
        //        0.0f,
        //        0.0f,
        //        1.0f
        //    }
        //);


        //nb::Node pointLight2 = scene.createNode();
        //pointLight2.setName("PointLight1");

        //auto& pl2Transform = pointLight2.getComponent<TransformComponent>();
        //pl2Transform.position = {5.0f, 0.0f, 5.0f};
        //pl2Transform.dirty = true;

        //pointLight2.addComponent(
        //    LightComponent{
        //        LightType::POINT,
        //        Colors::BLACK,
        //        Colors::WHITE,
        //        Color::fromLinearRgb(0.1f, 0.1f, 0.1f),
        //        {0.0f,0.0f,0.0f},
        //        1.0f,
        //        0.0f,
        //        0.0f,
        //        1.0f
        //    }
        //);


        //Ref<Mesh> cube = rm->getResource<Mesh>("sphere_exp.obj");

        //nb::Node cubeNode = scene.createNode();
        //cubeNode.setName("cube");

        //auto& cubeTransform = cubeNode.getComponent<TransformComponent>();
        //cubeTransform.position = {0.0f, 200.0f, 0.0f};
        //cubeTransform.scale = {1.0f, 1.0f, 1.0f};
        //cubeTransform.dirty = true;
        //cube->uniforms.shader = shader;
        //auto material = rm->getResource<nb::Resource::MaterialAsset>("Assets/res/plastic.material");
        //auto brickMaterial = rm->getResource<nb::Resource::MaterialAsset>("Assets/res/brick.material");
        //auto goldMaterial =
        //    rm->getResource<nb::Resource::MaterialAsset>("Assets/res/gold.material");

        //cubeNode.addComponent(MeshComponent{cube, {brickMaterial} });
        //auto aabb = cube->getAabb3d();
        //
        //cubeNode.addComponent(Physics::Collider { .halfSize = (aabb.halfSize() * 1.0f)  });

        ////cubeNode.addComponent(
        ////    Physics::Rigidbody{
        ////        .velocity = {0.0f, 0.0f, 0.0f},
        ////        .acceleration = {0.0f, 0.0f, 0.0f},
        ////        .mass = 1.0f,
        ////        .useGravity = true
        ////    }
        ////);
        //cubeNode.addComponent(
        //    nb::Script::ScriptComponent{
        //        .script = std::make_shared<nb::Script::Script>(
        //            nb::Script::ScriptEngineSingleton::instance(), "aa.lua"
        //        )
        //    }
        //);


        ////Ref<Mesh> surf = rm->getResource<Mesh>("reddd.obj");
        //Ref<Mesh> surf = rm->getResource<Mesh>("shader_ball_tri.obj");

        //nb::Node surfNode = scene.createNode();
        //surfNode.setName("scene");  

        //auto& surfTransform = surfNode.getComponent<TransformComponent>();
        //surfTransform.position = {0.0f, 0.0f, 0.0f};
        //surfTransform.scale = {0.1f, 0.1f, 0.1f};
        //surfTransform.dirty = true;
        //surf->uniforms.shader = shader;

        //surfNode.addComponent(
        //    MeshComponent{
        //        surf, {material, brickMaterial, goldMaterial, goldMaterial, goldMaterial, material}
        //    }
        //);
        ////surfNode.addComponent(Physics::Collider{.halfSize = {100.0f, 1.0f, 100.0f}});
        ////surfNode.addComponent(Physics::GroundTag());
        ////surfNode.addComponent(Physics::TerrainColliderComponent());
        ////auto bakedData = nb::Physics::bakeMesh(*surf, 0.1f);
        ////surfNode.getComponent<Physics::TerrainColliderComponent>().collider = std::move(bakedData);

        //


        ////surfNode.addComponent(nb::Script::ScriptComponent())

        //nb::Serialize::IArchive* archive =
        //    new nb::Serialize::JsonArchive("Assets/res/Scene.json");
        //Scene::getInstance().serialize(archive);
        //delete archive;
        //

        nb::Serialize::IArchive* archive =
            new nb::Serialize::JsonArchive("Assets/res/Scene.json");
        archive->setMode(nb::Serialize::JsonArchive::Mode::READ);
        archive->load();
        Scene::getInstance().deserialize(archive);
        delete archive;
    }
}
