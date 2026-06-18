#include "SceneController.hpp"
#include "../ComponentBrowser.hpp"
#include "../SceneModel.hpp"

#include <Error/ErrorConsolePrinter.hpp>
#include <Renderer/Scene.hpp>
#include <Serialize/JsonArchive.hpp>
#include <Utils/PrimitiveNameManager.hpp>

namespace sdk
{
    // Вспомогательная функция сборки примитивов
    [[nodiscard]] static Ref<nb::Renderer::Mesh> createPrimitiveMesh(
        std::string_view typeName,
        const void*      data
    ) noexcept
    {
        if (!data)
        {
            return nullptr;
        }

        if (typeName == "CubeParams")
        {
            return nb::Renderer::PrimitiveGenerators::createCube(
                static_cast<const CubeParams*>(data)->size
            );
        }
        if (typeName == "SphereParams")
        {
            const auto* params = static_cast<const SphereParams*>(data);
            return nb::Renderer::PrimitiveGenerators::createSphere(
                params->radius, params->xSegments, params->ySegments
            );
        }
        if (typeName == "TorusParams")
        {
            const auto* params = static_cast<const TorusParams*>(data);
            return nb::Renderer::PrimitiveGenerators::createTorus(
                {static_cast<uint32>(params->xSegments), static_cast<uint32>(params->ySegments)},
                params->majorRadius, params->minorRadius
            );
        }
        if (typeName == "CylinderParams")
        {
            const auto* params = static_cast<const CylinderParams*>(data);
            return nb::Renderer::PrimitiveGenerators::createCylinder(
                params->radius, params->height, params->xSegments, params->ySegments
            );
        }
        if (typeName == "PlaneParams")
        {
            const auto* params = static_cast<const PlaneParams*>(data);
            return nb::Renderer::PrimitiveGenerators::createPlane(
                params->width, params->height, params->xSegments, params->ySegments
            );
        }
        if (typeName == "ConeParams")
        {
            const auto* params = static_cast<const ConeParams*>(data);
            return nb::Renderer::PrimitiveGenerators::createCone(
                params->radius, params->height, params->radialSegments, params->heightSegments
            );
        }
        if (typeName == "PyramidParams")
        {
            const auto* params = static_cast<const PyramidParams*>(data);
            return nb::Renderer::PrimitiveGenerators::createPyramid(
                params->radius, params->height, params->sides
            );
        }
        return nullptr;
    }

    SceneController::SceneController(
        const std::shared_ptr<nb::Core::Engine>& enginePtr,
        const std::shared_ptr<SceneModelEcs>&    modelPtr,
        nb::Utils::PrimitiveNameManager&                    manager
    ) noexcept
        : engine(enginePtr)
        , sceneModel(modelPtr)
        , nameManager(manager)
    {
    }

    void SceneController::spawnPrimitive(
        const Widgets::ModelIndex&   index,
        const void*                  data,
        const nb::Reflect::TypeInfo* typeInfo
    ) noexcept
    {
        if (!index.isValid() || !data || !typeInfo)
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::WARNING, "Invalid spawn parameters")
                .with("hasData", data != nullptr)
                .with("hasType", typeInfo != nullptr);
            return;
        }

        auto* item = sceneModel->findById(index.getUuid());
        if (!item)
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::WARNING, "Could not find scene item by UUID")
                .with("uuid", index.getUuid().toString());
            return;
        }

        const auto parentId = reinterpret_cast<nb::Ecs::EntityID>(item->getData());
        auto&      scene    = nb::Scene::getInstance();

        const std::string_view  typeName = typeInfo->name;
        Ref<nb::Renderer::Mesh> mesh     = createPrimitiveMesh(typeInfo->name, data);

        if (mesh)
        {
            auto node = scene.createNode(parentId);

            node.addComponent<MeshComponent>({.mesh = mesh, .material = {}});
            std::string primitiveName = nameManager.generateName(typeName);
            node.addComponent<NameComponent>({primitiveName});
            node.addComponent<TransformComponent>({});

            sceneModel->addEntity(parentId, node.getId());

            activeNode = scene.getNode(node.getId());
            refreshHierarchySignal.emit();
            activeNodeChanged.emit();
            scene.invalidateBvh();

            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::INFO, "Primitive spawned successfully")
                .with("type", typeInfo->name)
                .with("name", primitiveName)
                .with("entityId", static_cast<uint64_t>(node.getId()));
        }
        else
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::FATAL, "Failed to create mesh for primitive")
                .with("type", typeInfo->name);
        }
    }

    void SceneController::spawnEmpty(const Widgets::ModelIndex& index) noexcept
    {
        if (!index.isValid())
        {
            nb::Error::ErrorManager::instance().report(
                nb::Error::Type::WARNING, "Invalid spawn parameters for empty node"
            );
            return;
        }

        auto* item = sceneModel->findById(index.getUuid());
        if (!item)
        {
            nb::Error::ErrorManager::instance()
                .report(
                    nb::Error::Type::WARNING, "Could not find scene item by UUID for empty node"
                )
                .with("uuid", index.getUuid().toString());
            return;
        }

        const auto parentId = reinterpret_cast<nb::Ecs::EntityID>(item->getData());
        auto&      scene    = nb::Scene::getInstance();
        auto       node     = scene.createNode(parentId);

        std::string nodeName = nameManager.generateName("Empty");
        node.addComponent<NameComponent>({nodeName});
        node.addComponent<TransformComponent>({});

        sceneModel->addEntity(parentId, node.getId());

        activeNode = scene.getNode(node.getId());

        refreshHierarchySignal.emit();
        activeNodeChanged.emit();

        scene.invalidateBvh();

        nb::Error::ErrorManager::instance()
            .report(nb::Error::Type::INFO, "Empty node spawned successfully")
            .with("name", nodeName)
            .with("entityId", static_cast<uint64_t>(node.getId()));
    }

    void SceneController::spawnModel(
        const Widgets::ModelIndex&      index,
        const std::filesystem::path&    pathToModel,
        const nb::Math::Vector3<float>& position
    ) noexcept
    {
        nb::Ecs::EntityID parentId = sceneModel->getRoot();

        auto& scene = nb::Scene::getInstance();
        auto  node  = scene.createNode(parentId);

        std::vector<Ref<nb::Resource::MaterialAsset>> materials;
        std::string                                   meshResourcePath = pathToModel.string();

        if (std::filesystem::exists(pathToModel) && pathToModel.extension() == ".model")
        {
            auto modelJson = nb::Loaders::Json(pathToModel);

            if (modelJson.contains("mesh_source"))
            {
                meshResourcePath = modelJson["mesh_source"].get<std::string>();
            }

            if (modelJson.contains("submeshes"))
            {
                auto submeshes = modelJson["submeshes"];
                for (size_t i = 0; i < submeshes.size(); ++i)
                {
                    if (submeshes[i].contains("material"))
                    {
                        std::string matPath = submeshes[i]["material"].get<std::string>();
                        auto        res = nb::ResMan::ResourceManager::getInstance()
                                              ->getResource<nb::Resource::MaterialAsset>(matPath);
                        if (res)
                        {
                            materials.push_back(res);
                        }
                    }
                }
            }
        }

        std::string nodeName = nameManager.generateName(pathToModel.filename().string());
        node.addComponent<NameComponent>({nodeName});
        node.addComponent<TransformComponent>(TransformComponent{.position = position});

        node.addComponent<MeshComponent>(
            {.mesh = nb::ResMan::ResourceManager::getInstance()->getResource<nb::Renderer::Mesh>(
                 pathToModel.string()
             ),
             .material = materials}
        );

        sceneModel->addEntity(parentId, node.getId());
        activeNode = scene.getNode(node.getId());

        refreshHierarchySignal.emit();
        activeNodeChanged.emit();

        scene.invalidateBvh();

        nb::Error::ErrorManager::instance()
            .report(nb::Error::Type::INFO, "Model spawned successfully")
            .with("name", nodeName);
    }

    void SceneController::deleteEntity(const Widgets::ModelIndex& index) noexcept
    {
        if (!index.isValid())
        {
            return;
        }

        nb::Ecs::EntityID id    = sceneModel->getEntity(index);
        auto&             scene = nb::Scene::getInstance();

        releaseNamesRecursive(id);
        sceneModel->removeEntity(id);
        scene.deleteEntity(id);

        activeNode = nb::Node::createInvalid();

        refreshHierarchySignal.emit();
        activeNodeChanged.emit();
        scene.invalidateBvh();

        nb::Error::ErrorManager::instance()
            .report(nb::Error::Type::INFO, "Entity deleted successfully")
            .with("entityId", static_cast<uint64_t>(id));
    }

    void SceneController::copyEntity(const Widgets::ModelIndex& index) noexcept
    {
        if (!index.isValid())
        {
            return;
        }

        nb::Ecs::EntityID id = sceneModel->getEntity(index);
        copiedEntityId       = id;

        nb::Error::ErrorManager::instance()
            .report(nb::Error::Type::INFO, "Entity copied to clipboard")
            .with("entityId", static_cast<uint64_t>(id));
    }

    void SceneController::pasteEntity(const Widgets::ModelIndex& index) noexcept
    {
        if (!index.isValid() || copiedEntityId == 0)
        {
            return;
        }

        nb::Ecs::EntityID id    = sceneModel->getEntity(index);
        auto&             scene = nb::Scene::getInstance();

        nb::Node       copy     = scene.clone(id, copiedEntityId);
        NameComponent& nameComp = copy.getComponent<NameComponent>();
        nameComp.name += " (Copy)";
        nameComp.name = nameManager.generateName(nameComp.name);

        sceneModel->addEntity(id, copy.getId());

        activeNode = copy;

        refreshHierarchySignal.emit();
        activeNodeChanged.emit();
        scene.invalidateBvh();

        nb::Error::ErrorManager::instance()
            .report(nb::Error::Type::INFO, "Entity pasted successfully")
            .with("newEntityId", static_cast<uint64_t>(copy.getId()));
    }

    void SceneController::copyEntityById(nb::Ecs::EntityID id) noexcept
    {
        copiedEntityId       = id;

        nb::Error::ErrorManager::instance()
            .report(nb::Error::Type::INFO, "Entity copied to clipboard")
            .with("entityId", static_cast<uint64_t>(id));
    }

    void SceneController::pasteEntityById(nb::Ecs::EntityID id) noexcept
    {
        if (copiedEntityId == 0)
        {
            return;
        }

        auto&             scene = nb::Scene::getInstance();

        nb::Node       copy     = scene.clone(id, copiedEntityId);
        NameComponent& nameComp = copy.getComponent<NameComponent>();
        nameComp.name += " (Copy)";
        nameComp.name = nameManager.generateName(nameComp.name);

        sceneModel->addEntity(id, copy.getId());

        activeNode = copy;

        refreshHierarchySignal.emit();
        activeNodeChanged.emit();
        scene.invalidateBvh();

        nb::Error::ErrorManager::instance()
            .report(nb::Error::Type::INFO, "Entity pasted successfully")
            .with("newEntityId", static_cast<uint64_t>(copy.getId()));
    }

    void SceneController::releaseNamesRecursive(nb::Ecs::EntityID id) noexcept
    {
        auto& scene = nb::Scene::getInstance();

        if (scene.hasComponent<NameComponent>(id))
        {
            const std::string& name = scene.getComponent<NameComponent>(id).name;
            nameManager.releaseName(name);
        }

        if (scene.hasComponent<HierarchyComponent>(id))
        {
            const auto& hierarchy = scene.getComponent<HierarchyComponent>(id);
            for (auto childId : hierarchy.children)
            {
                releaseNamesRecursive(childId);
            }
        }
    }

    void SceneController::markComponentDirty(
        void*                        componentPtr,
        const nb::Reflect::TypeInfo* typeInfo
    ) noexcept
    {
        if (!componentPtr || !typeInfo)
        {
            return;
        }

        for (const auto& field : typeInfo->fields)
        {
            const std::string_view fieldName = field.name;
            if (fieldName == "dirty" || fieldName == "physicsDirty")
            {
                void* bytePtr  = static_cast<char*>(componentPtr) + field.offset;
                bool* dirtyPtr = reinterpret_cast<bool*>(bytePtr);

                *dirtyPtr = true;
                return;
            }
        }
    }

} // namespace sdk