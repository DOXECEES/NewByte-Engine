#include "Scene.hpp"

#include <Math/Matrix/Transformation.hpp>
#include "Renderer/Light.hpp"

#include "Serialize/JsonArchive.hpp"

#include "Manager/ResourceManager.hpp"

#include "Scripting/ScriptComponent.hpp"
#include "Physics/Physics.hpp"

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
        return entity != INVALID_VALUE;
    }

    Node Node::createInvalid() noexcept
    {
        return Node(INVALID_VALUE, nullptr);
    }

    std::optional<Node> Node::getParent() noexcept
    {
        if (!hasComponent<HierarchyComponent>())
        {
            return std::nullopt;
        }

        auto& hierarchy = getComponent<HierarchyComponent>();
        return Node(hierarchy.parent, scene);

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

    void Scene::deleteEntity(Ecs::EntityID id) noexcept
    {
        if (hasComponent<HierarchyComponent>(id))
        {
            Ecs::EntityID parentId = getComponent<HierarchyComponent>(id).parent;
            if (parentId != 0 && hasComponent<HierarchyComponent>(parentId))
            {
                auto& parentHierarchy = getComponent<HierarchyComponent>(parentId);
                auto& children        = parentHierarchy.children;

                children.erase(std::remove(children.begin(), children.end(), id), children.end());
            }
        }

        if (hasComponent<HierarchyComponent>(id))
        {
            auto childrenCopy = getComponent<HierarchyComponent>(id).children;


            for (auto childId : childrenCopy)
            {
                deleteEntity(childId);
            }
        }

        ecs.destroyEntity(Ecs::Entity{id});
    }

    Node Scene::findNodeByName(std::string_view name) noexcept
    {
        auto entities = getEntitiesWith<NameComponent>();

        for (auto& entity : entities)
        {
            auto& nameComp = ecs.get<NameComponent>(entity);
            if (nameComp.name == name)
            {
                return Node(entity.id, this);
            }
        }

        return Node::createInvalid();
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
        Ecs::EntityID                entityId,
        const nb::Math::Mat4<float>& parentTransform,
        bool                         parentDirty
    ) noexcept
    {
        auto& transform = ecs.get<TransformComponent>(Ecs::Entity{entityId});
        auto& hierarchy = ecs.get<HierarchyComponent>(Ecs::Entity{entityId});

        if (transform.eulerAngle.x != transform.lastEuler.x ||
            transform.eulerAngle.y != transform.lastEuler.y ||
            transform.eulerAngle.z != transform.lastEuler.z)
        {
            transform.rotation = nb::Math::Quaternion<float>::eulerToQuaternionXYZ(
                transform.eulerAngle.x, transform.eulerAngle.y, transform.eulerAngle.z
            );

            transform.rotation.normalize();
            transform.lastEuler = transform.eulerAngle;
            transform.dirty     = true;
        }

        bool isDirty = transform.dirty || parentDirty;

        if (isDirty)
        {
            transform.rotation.normalize();

            nb::Math::Mat4<float> local = nb::Math::Mat4<float>::identity();

            // S * R * T
            local = nb::Math::scale(local, transform.scale);
            local = local * transform.rotation.toMatrix4();
            local = nb::Math::translate(local, transform.position);

            transform.localMatrix = local;
            transform.worldMatrix = local * parentTransform;

            transform.dirty = false;
            invalidateBvh();
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

    Ecs::EntityID Scene::pickNode(const Math::Ray& ray) noexcept
    {
        updateBvh();


        uint32_t stack[64];
        uint32_t stackPtr = 0;
        uint32_t currentNodeIdx = 0;

        Ecs::EntityID closestEntity = 0;
        float closestT = std::numeric_limits<float>::max();


        while (true)
        {
            const auto& node = sceneBVH.nodes[currentNodeIdx];

            float tNode;
            if (!intersectRayAABB(ray, node.bounds, tNode) || tNode > closestT)
            {
                if (stackPtr == 0)
                {
                    break;
                }
                currentNodeIdx = stack[--stackPtr];
                continue;
            }

            if (node.isLeaf())
            {
                for (uint32_t i = 0; i < node.count; i++)
                {
                    const auto& item = sceneBVH.items[node.leftFirst + i];

                    float tAABB;
                    if (intersectRayAABB(ray, item.worldAABB, tAABB) && tAABB < closestT)
                    {
                        auto& transform = getComponent<TransformComponent>(item.entityId);
                        auto& meshComp = getComponent<MeshComponent>(item.entityId);

                        Math::Mat4<float> invModel = Math::inverse(transform.worldMatrix);

                        Math::Ray localRay;
                        localRay.origin = Math::transformPoint(invModel, ray.origin);

                        localRay.direction = Math::transformVector(invModel, ray.direction);

                        const auto& vertices = meshComp.mesh->getVertices();
                        const auto& indices = meshComp.mesh->getIndices();

                        for (size_t j = 0; j < indices.size(); j += 3)
                        {
                            float tTri;
                            if (Math::intersectRayTriangle(
                                    localRay, vertices[indices[j]].position,
                                    vertices[indices[j + 1]].position,
                                    vertices[indices[j + 2]].position, tTri
                                ))
                            {
                                if (tTri < closestT && tTri > 0.0001f)
                                {
                                    closestT = tTri;
                                    closestEntity = item.entityId;
                                }
                            }
                        }
                    }
                }

                if (stackPtr == 0)
                {
                    break;
                }
                currentNodeIdx = stack[--stackPtr];
            }
            else
            {
                uint32_t leftIdx = node.leftFirst;
                uint32_t rightIdx = node.leftFirst + 1;

                float tLeft, tRight;
                bool hitLeft = Math::intersectRayAABB(ray, sceneBVH.nodes[leftIdx].bounds, tLeft);
                bool hitRight = Math::intersectRayAABB(ray, sceneBVH.nodes[rightIdx].bounds, tRight);

                if (hitLeft && tLeft > closestT)
                {
                    hitLeft = false;
                }
                if (hitRight && tRight > closestT)
                {
                    hitRight = false;
                }

                if (!hitLeft && !hitRight)
                {
                    if (stackPtr == 0)
                    {
                        break;
                    }
                    currentNodeIdx = stack[--stackPtr];
                }
                else if (hitLeft && !hitRight)
                {
                    currentNodeIdx = leftIdx;
                }
                else if (!hitLeft && hitRight)
                {
                    currentNodeIdx = rightIdx;
                }
                else
                {
                    if (tLeft < tRight)
                    {
                        currentNodeIdx = leftIdx;
                        stack[stackPtr++] = rightIdx;
                    }
                    else
                    {
                        currentNodeIdx = rightIdx;
                        stack[stackPtr++] = leftIdx;
                    }
                }
            }
        }

        return closestEntity;
    }

    void Scene::updateBvh() noexcept
    {
        if (!bvhDirty)
        {
            return;
        }

        auto ent = getEntitiesWith<MeshComponent, TransformComponent>();

        std::vector<Math::BVHItem> items;
        items.reserve(ent.size());

        for (auto& entity : ent)
        {
            auto& transform = getComponent<TransformComponent>(entity.id);
            auto& meshComp = getComponent<MeshComponent>(entity.id);

            Math::AABB3D worldAABB = Math::AABB3D::recalculateAabb3dByModelMatrix(
                meshComp.mesh->getAabb3d(), transform.worldMatrix
            );

            items.push_back({entity.id, worldAABB});
        }

        if (items.empty())
        {
            sceneBVH.clear(); 
        }
        else
        {
            sceneBVH.build(std::move(items));
        }

        bvhDirty = false;
    }

    void Scene::invalidateBvh() noexcept
    {
        bvhDirty = true;
    }

    Math::BVH* Scene::getBvh() noexcept
    {
        return &sceneBVH;
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
        ecs.getStorage<nb::Script::ScriptComponent>();


        ecs.getStorage<nb::Renderer::LightComponent>();
        ecs.getStorage<nb::Physics::Collider>();
//        ecs.getStorage<nb::Physics::GroundTag>();
        ecs.getStorage<nb::Physics::Rigidbody>();
        ecs.getStorage<CameraComponent>();

        //ecs.getStorage<nb::Physics::TerrainColliderComponent>();


        //nb::Serialize::IArchive* archive =
        //    new nb::Serialize::PseudoJsonArchive("Assets/res/Scene.json");
        //serialize(archive);
        //delete archive;
    }

    void Scene::clear() noexcept
    {
        ecs.clear();

        sceneBVH = Math::BVH();
        bvhDirty = true;

        auto root  = ecs.createEntity();
        rootEntity = root.id;

        ecs.add(root, TransformComponent{});
        ecs.add(root, HierarchyComponent{0});
        ecs.add(root, NameComponent{"Root"});

        ecs.getStorage<TransformComponent>();
        ecs.getStorage<HierarchyComponent>();
        ecs.getStorage<NameComponent>();
        ecs.getStorage<MeshComponent>();
        ecs.getStorage<nb::Script::ScriptComponent>();

        ecs.getStorage<nb::Renderer::LightComponent>();
        ecs.getStorage<nb::Physics::Collider>();
        //ecs.getStorage<nb::Physics::GroundTag>();
        ecs.getStorage<nb::Physics::Rigidbody>();
        ecs.getStorage<CameraComponent>();

        //ecs.getStorage<nb::Physics::TerrainColliderComponent>();
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

            if (!strcmp(type->name, "HierarchyComponent"))
            {
                continue;
            }

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
                archive->beginObject(type->name);
                
                archive->endObject();
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
                bool handledAsComplex =
                    dispatcher.serialize(archive, field.name, fieldPtr, field.type);

                if (!handledAsComplex)
                {
                    std::string path = field.getResourcePath(fieldPtr);
                    archive->value(field.name, path);
                }
                continue;

            }
            else if (field.getResourcePaths)
            {
                std::vector<std::string> paths = field.getResourcePaths(fieldPtr);
                archive->beginArray(field.name);
                for (auto& path : paths)
                {
                    archive->value(nullptr, path);
                }
                archive->endArray();
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

            storage->addDefault(entity.id);

            void* componentPtr = storage->getRaw(entity.id);

            if (componentPtr)
            {
                deserializeFields(compJson, componentPtr, typeInfo);
            }

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
            else if (fieldType->isEnum && fieldJson.isValue())
            {
                int value = fieldJson.get<int>();
                std::memcpy(fieldPtr, &value, sizeof(int));
            }
            else if (field.loadResource)
            {
                if (fieldJson.isValue())
                {
                    field.loadResource(fieldPtr, fieldJson.get<std::string>());
                }
                else if (fieldJson.isObject())
                {
                    
                    dispatcher.dispatch(fieldPtr, field.type, fieldJson);
                    
                }
            }

            else if (field.loadResources && fieldJson.isArray())
            {
                std::vector<std::string> paths;
                for (size_t i = 0; i < fieldJson.size(); ++i)
                {
                    if (fieldJson[i].isValue())
                    {
                        paths.push_back(fieldJson[i].get<std::string>());
                    }
                }
                field.loadResources(fieldPtr, paths);
            }
            else if (!fieldType->fields.empty() && fieldJson.isObject())
            {
                deserializeFields(fieldJson, fieldPtr, fieldType);
            }
        }
    }
};
