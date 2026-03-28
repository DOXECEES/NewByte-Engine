#include "Scene.hpp"

#include <Math/Matrix/Transformation.hpp>
#include "Renderer/Light.hpp"

#include "Serialize/JsonArchive.hpp"

#include "Manager/ResourceManager.hpp"

namespace nb
{
    Node::Node() noexcept
    {
        entity = ~uint32_t(0);
    }

    Node::Node(
        Ecs::EntityID id,
        Scene* scene
    ) noexcept
        : entity(id),
          scene(scene)
    {
    }

    Ecs::EntityID Node::getId() const noexcept
    {
        return entity;
    }

    void Node::setName(std::string_view name)
    {
        scene->addComponent(entity, NameComponent{std::string(name)});
    }

    bool Node::isValid() const noexcept
    {
        return entity != ~uint32_t(0);
    }

    Node Scene::createNode(Ecs::EntityID parent) noexcept
    {
        auto entity = ecs.createEntity();

        if (parent == 0)
        {
            parent = rootEntity;
        }

        ecs.add(entity, TransformComponent{});
        ecs.add(entity, HierarchyComponent{parent});

        HierarchyComponent& hierarchy = ecs.get<HierarchyComponent>(entity);
        hierarchy.parent = parent;

        if (parent != 0)
        {
            auto& parentHierarchy = ecs.get<HierarchyComponent>(Ecs::Entity{parent});

            parentHierarchy.children.push_back(entity.id);
        }

        return Node{entity.id, this};
    }

    Node Scene::getNode(Ecs::EntityID id) noexcept
    {
        return Node{id, this};
    }

    Ecs::ECSRegistry& Scene::getRegistry() noexcept
    {
        return ecs;
    }

    Ecs::Entity Scene::getRootEntity() noexcept
    {
        return Ecs::Entity{rootEntity};
    }

    void Scene::updateWorldTransform(
        Ecs::EntityID entityId,
        const nb::Math::Mat4<float>& parentTransform,
        bool parentDirty
    ) noexcept
    {
        auto& transform = ecs.get<TransformComponent>(Ecs::Entity{entityId});
        auto& hierarchy = ecs.get<HierarchyComponent>(Ecs::Entity{entityId});

        bool isDirty = transform.dirty || parentDirty;

        if (isDirty)
        {
            nb::Math::Mat4<float> local = nb::Math::Mat4<float>::identity();

            local = Math::scale(local, transform.scale);
            local = Math::rotate(local, {1.0f, 0.0f, 0.0f}, transform.rotation.x);
            local = Math::rotate(local, {0.0f, 1.0f, 0.0f}, transform.rotation.y);
            local = Math::rotate(local, {0.0f, 0.0f, 1.0f}, transform.rotation.z);
            local = Math::translate(local, transform.position);

            transform.localMatrix = local;

            transform.worldMatrix = parentTransform * transform.localMatrix;

            transform.dirty = false;
        }

        for (auto childId : hierarchy.children)
        {
            updateWorldTransform(childId, transform.worldMatrix, isDirty);
        }
    }

    void Scene::traverse(
        Ecs::EntityID entityId,
        const std::function<void(Ecs::EntityID)>& action
    ) noexcept
    {
        action(entityId);
        auto& hierarchy = ecs.get<HierarchyComponent>(Ecs::Entity{entityId});

        for (auto childId : hierarchy.children)
        {
            traverse(childId, action);
        }
    }

    void Scene::updateAllTransforms() noexcept
    {
        updateWorldTransform(rootEntity, nb::Math::Mat4<float>::identity(), false);
    }

    void Scene::traverseAll(const std::function<void(Ecs::EntityID)>& action) noexcept
    {
        traverse(rootEntity, action);
    }


    Scene::Scene() noexcept
    {
        auto root = ecs.createEntity();
        rootEntity = root.id;

        ecs.add(root, TransformComponent{});
        ecs.add(root, HierarchyComponent{0});
        ecs.add(root, NameComponent{"Root"});


        ecs.getStorage<TransformComponent>();
        ecs.getStorage<HierarchyComponent>();
        ecs.getStorage<NameComponent>();
        ecs.getStorage<MeshComponent>();


        ecs.getStorage<nb::Renderer::LightComponent>();
        //nb::Serialize::IArchive* archive =
        //    new nb::Serialize::PseudoJsonArchive("Assets/res/Scene.json");
        //serialize(archive);
        //delete archive;
    }

    void Scene::serialize(nb::Serialize::IArchive* archive) noexcept
    {
        if (!archive)
        {
            return;
        }

        archive->setMode(nb::Serialize::IArchive::Mode::WRITE);
        archive->beginObject("");
        uint32_t magic = 0x53434E45;
        archive->value("magic", magic);

        uint32_t version = 2;
        archive->value("version", version);

        archive->beginArray("nodes");

        std::function<void(Ecs::EntityID, int32_t)> serializeNodeRec;
        serializeNodeRec = [&](Ecs::EntityID id, int32_t depth)
        {
            Ecs::Entity entity{id};

            // элемент массива
            archive->beginObject(nullptr);

            archive->value("depth", depth);

            serializeComponents(archive, entity);

            auto& hierarchy = ecs.get<HierarchyComponent>(entity);
            if (!hierarchy.children.empty())
            {
                archive->beginArray("children");

                for (auto childId : hierarchy.children)
                {
                    serializeNodeRec(childId, depth + 1);
                }

                archive->endArray();
            }

            archive->endObject();
        };

        serializeNodeRec(rootEntity, 0);

        archive->endArray();
        archive->endObject();
        archive->save();
    }

    void Scene::serializeComponents(
        nb::Serialize::IArchive* archive,
        Ecs::Entity entity
    ) noexcept
    {
        if (!archive)
        {
            return;
        }

        archive->beginObject("components");

        bool hasAnyComponents = false;

        for (auto& storage : ecs.getAllStorages())
        {
            if (!storage || !storage->contains(entity.id))
            {
                continue;
            }

            nb::Reflect::TypeInfo* type = storage->getTypeInfo();
            if (!type)
            {
                continue;
            }

            void* component = storage->getRaw(entity.id);
            if (!component)
            {
                continue;
            }

            // Проверяем, есть ли у компонента поля для сериализации
            bool hasFields = false;
            for (auto& field : type->fields)
            {
                if (field.visibleIf && !field.visibleIf(component))
                {
                    continue;
                }
                hasFields = true;
                break;
            }

            if (!hasFields)
            {
                continue; // Пропускаем компоненты без полей
            }

            archive->beginObject(type->name);
            serializeFields(archive, component, type);
            archive->endObject();

            hasAnyComponents = true;
        }

        if (!hasAnyComponents)
        {
            
        }

        archive->endObject();
    }

    void Scene::serializeFields(
        nb::Serialize::IArchive* archive,
        void* object,
        nb::Reflect::TypeInfo* type
    ) noexcept
    {
        if (!archive || !object || !type)
        {
            return;
        }

        // Проверяем, есть ли вообще поля для сериализации
        bool hasFields = false;
        for (auto& field : type->fields)
        {
            if (field.visibleIf && !field.visibleIf(object))
            {
                continue;
            }
            hasFields = true;
            break;
        }

        // Если полей нет, ничего не делаем
        if (!hasFields)
        {
            return;
        }

        for (auto& field : type->fields)
        {
            if (field.visibleIf && !field.visibleIf(object))
            {
                continue;
            }

            void* fieldPtr = reinterpret_cast<uint8_t*>(object) + field.offset;

            if (field.getResourcePath)
            {
                std::string path = field.getResourcePath(fieldPtr);
                archive->value(field.name, path);
                continue;
            }
            else if (std::strcmp(field.type->name, "float") == 0)
            {
                archive->value(field.name, *reinterpret_cast<float*>(fieldPtr));
            }
            else if (std::strcmp(field.type->name, "int") == 0)
            {
                archive->value(field.name, *reinterpret_cast<int*>(fieldPtr));
            }
            else if (std::strcmp(field.type->name, "int32_t") == 0)
            {
                archive->value(field.name, *reinterpret_cast<int32_t*>(fieldPtr));
            }
            else if (std::strcmp(field.type->name, "uint32_t") == 0)
            {
                archive->value(field.name, *reinterpret_cast<uint32_t*>(fieldPtr));
            }
            else if (std::strcmp(field.type->name, "bool") == 0)
            {
                archive->value(field.name, *reinterpret_cast<bool*>(fieldPtr));
            }
            else if (std::strcmp(field.type->name, "std::string") == 0)
            {
                archive->value(field.name, *reinterpret_cast<std::string*>(fieldPtr));
            }
            else if (std::strcmp(field.type->name, "uint8_t") == 0)
            {
                archive->value(field.name, *reinterpret_cast<uint8_t*>(fieldPtr));
            }
            //if (nb::Reflect::ResourceLoaderBase::instance().hasLoader(field.type))
            //{

            //}
            else if (field.type->isEnum)
            {
                archive->value(field.name, *reinterpret_cast<int*>(fieldPtr));
            }
            else if (!field.type->fields.empty())
            {
                archive->beginObject(field.name);
                serializeFields(archive, fieldPtr, field.type);
                archive->endObject();
            }
            
        }
    }



    void Scene::addComponentRaw(
        Ecs::Entity entity,
        Ecs::StorageWrapperBase* storage,
        void* component
    ) noexcept
    {
        if (storage)
        {
            storage->addRaw(entity.id, component);
        }
    }


    void Scene::deserialize(nb::Serialize::IArchive* archive) noexcept
    {
        if (!archive)
        {
            return;
        }

        archive->setMode(nb::Serialize::IArchive::Mode::READ);

        /*if (!archive->load())
        {
            NB_ERROR("Failed to load scene file");
            return;
        }*/

        // Получаем корневой узел из архива
        auto* jsonArchive = dynamic_cast<nb::Serialize::JsonArchive*>(archive);
        if (!jsonArchive)
        {
            //NB_ERROR("Invalid archive type");
            return;
        }

        const auto& root = jsonArchive->getJson();

        // Проверяем магическое число
        if (/*!root.isObject() ||*/ !root["magic"].isValue() ||
            root["magic"].get<int>() != 0x53434E45)
        {
            //NB_ERROR("Invalid scene file format");
            return;
        }

        uint32_t version = root["version"].get<int>();
        if (version != 2)
        {
            //NB_ERROR("Unsupported scene version: %u", version);
            return;
        }

        // Очищаем сцену
        //clearExceptRoot();

        // Проверяем наличие узлов
        if (!root["nodes"].isArray())
        {
            return;
        }

        const auto& nodes = root["nodes"];

        // Стек для восстановления иерархии
        struct NodeInfo
        {
            Ecs::Entity entity;
            int32_t depth;
        };
        std::vector<NodeInfo> nodeStack;

        // Рекурсивная функция для обхода узлов
        std::function<void(const nb::Loaders::Node&, int32_t)> deserializeNode;
        deserializeNode = [&](const nb::Loaders::Node& nodeJson, int32_t depth)
        {
            Ecs::Entity entity = ecs.createEntity();

            ecs.add(entity, TransformComponent{});
            ecs.add(entity, HierarchyComponent{});

            deserializeComponents(nodeJson, entity);

            if (depth == 0)
            {
                entity = Ecs::Entity{rootEntity};

            }
            else
            {
                while (!nodeStack.empty() && nodeStack.back().depth >= depth)
                {
                    nodeStack.pop_back();
                }

                if (!nodeStack.empty())
                {
                    auto& parentHierarchy = ecs.get<HierarchyComponent>(nodeStack.back().entity);
                    auto& hierarchy = ecs.get<HierarchyComponent>(entity);

                    hierarchy.parent = nodeStack.back().entity.id;
                    parentHierarchy.children.push_back(entity.id);
                }
            }

            nodeStack.push_back({entity, depth});

            if (nodeJson.contains("children") && nodeJson["children"].isArray())
            {
                for (size_t i = 0; i < nodeJson["children"].size(); ++i)
                {
                    deserializeNode(nodeJson["children"][i], depth + 1);
                }
            }
        };

        for (size_t i = 0; i < nodes.size(); ++i)
        {
            const auto& nodeJson = nodes[i];
            int32_t depth = nodeJson["depth"].get<int32_t>();
            deserializeNode(nodeJson, depth);
        }

        updateAllTransforms();
    }

    void Scene::deserializeComponents(
        const nb::Loaders::Node& node,
        Ecs::Entity entity
    ) noexcept
    {
        if (!node.isObject() || !node["components"].isObject())
        {
            return;
        }

        const auto& components = node["components"];

        const auto& storages = ecs.getAllStorages();

        for (const auto& storage : storages)
        {
            if (!storage)
            {
                continue;
            }

            nb::Reflect::TypeInfo* typeInfo = storage->getTypeInfo();
            if (!typeInfo)
            {
                continue;
            }

            if (!components.isObject() || !components.contains(typeInfo->name) ||
                !components[typeInfo->name].isObject())
            {
                continue;
            }

            const auto& compJson = components[typeInfo->name];

            void* tempComponent = malloc(typeInfo->size);
            memset(tempComponent, 0, typeInfo->size);

            deserializeFields(compJson, tempComponent, typeInfo);

            addComponentRaw(entity, storage.get(), tempComponent);

            free(tempComponent);
        }
    }

void Scene::deserializeFields(
        const nb::Loaders::Node& node,
        void* object,
        nb::Reflect::TypeInfo* type
    ) noexcept
    {
        if (!object || !type)
        {
            return;
        }

        for (auto& field : type->fields)
        {
            if (!node.isObject() || !node.contains(field.name))
            {
                continue;
            }

            const auto& fieldJson = node[field.name];
            void* fieldPtr = reinterpret_cast<uint8_t*>(object) + field.offset;
            auto* fieldType = field.type;
            const char* typeName = fieldType->name;

            // ---- Примитивы ----
            if (std::strcmp(typeName, "float") == 0 && fieldJson.isValue())
            {
                *reinterpret_cast<float*>(fieldPtr) = fieldJson.get<float>();
            }
            else if (
                (std::strcmp(typeName, "int") == 0 || std::strcmp(typeName, "int32_t") == 0) &&
                fieldJson.isValue()
            )
            {
                *reinterpret_cast<int32_t*>(fieldPtr) = fieldJson.get<int32_t>();
            }
            else if (std::strcmp(typeName, "uint32_t") == 0 && fieldJson.isValue())
            {
                *reinterpret_cast<uint32_t*>(fieldPtr) = fieldJson.get<uint32_t>();
            }
            else if (std::strcmp(typeName, "bool") == 0 && fieldJson.isValue())
            {
                *reinterpret_cast<bool*>(fieldPtr) = fieldJson.get<bool>();
            }
            else if (std::strcmp(typeName, "std::string") == 0 && fieldJson.isValue())
            {
                *reinterpret_cast<std::string*>(fieldPtr) = fieldJson.get<std::string>();
            }
            else if (std::strcmp(typeName, "uint8_t") == 0 && fieldJson.isValue())
            {
                *reinterpret_cast<uint8_t*>(fieldPtr) = static_cast<uint8_t>(fieldJson.get<int>());
            }

            // ---- Enum ----
            else if (fieldType->isEnum && fieldJson.isValue())
            {
                int value = fieldJson.get<int>();
                std::memcpy(fieldPtr, &value, sizeof(int));
            }

            // ---- Ресурсы ----
            else if (fieldJson.isValue())
            {
                std::string path = fieldJson.get<std::string>();
                if (std::strcmp(typeName, "std::shared_ptr<nb::Renderer::Mesh>") == 0)
                {
                    *reinterpret_cast<std::shared_ptr<nb::Renderer::Mesh>*>(fieldPtr) =
                        nb::ResMan::ResourceManager::getInstance()->getResource<nb::Renderer::Mesh>(
                            path
                        );
                }
                else if (std::strcmp(typeName, "Ref<nb::Renderer::Material>") == 0)
                {
                    //*reinterpret_cast<Ref<nb::Renderer::Material>*>(fieldPtr) =
                    //    nb::ResMan::ResourceManager::getInstance()
                    //        ->getResource<nb::Renderer::Material>(path);
                }
            }

            // ---- Структуры с полями ----
            else if (!fieldType->fields.empty() && fieldJson.isObject())
            {
                deserializeFields(fieldJson, fieldPtr, fieldType);
            }
        }
    }
};
