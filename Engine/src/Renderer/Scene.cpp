#include "Scene.hpp"

#include <Math/Matrix/Transformation.hpp>
#include "Renderer/Light.hpp"

#include "Serialize/JsonArchive.hpp"

#include "Manager/ResourceManager.hpp"

#include "Scripting/ScriptComponent.hpp"

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
    using namespace Math;
    bool intersectRayTriangle(
        const Ray& ray,
        const Vector3<float>& v0,
        const Vector3<float>& v1,
        const Vector3<float>& v2,
        float& t
    )
    {
        const float EPSILON = 1e-7f;
        Vector3<float> edge1 = v1 - v0;
        Vector3<float> edge2 = v2 - v0;
        Vector3<float> h = ray.direction.cross(edge2);
        float a = edge1.dot(h);

        if (a > -EPSILON && a < EPSILON)
        {
            return false; // Луч параллелен
        }

        float f = 1.0f / a;
        Vector3<float> s = ray.origin - v0;
        float u = f * s.dot(h);

        if (u < 0.0f || u > 1.0f)
        {
            return false;
        }

        Vector3<float> q = s.cross(edge1);
        float v = f * ray.direction.dot(q);

        if (v < 0.0f || u + v > 1.0f)
        {
            return false;
        }

        float tempT = f * edge2.dot(q);
        if (tempT > EPSILON)
        {
            t = tempT;
            return true;
        }
        return false;
    }

    inline Vector3<float> transformPoint(
        const Mat4<float>& m,
        const Vector3<float>& v
    )
    {
        float x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3];
        float y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3];
        float z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3];
        float w = m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3];
        return Vector3<float>(x / w, y / w, z / w);
    }

    // Умножение вектора на матрицу 4x4 (w = 0, без переноса)
    inline Vector3<float> transformVector(
        const Mat4<float>& m,
        const Vector3<float>& v
    )
    {
        float x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z;
        float y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z;
        float z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z;
        return Vector3<float>(x, y, z);
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

                        Mat4<float> invModel = Math::inverse(transform.worldMatrix);

                        Math::Ray localRay;
                        localRay.origin = transformPoint(invModel, ray.origin);

                        localRay.direction = transformVector(invModel, ray.direction);

                        const auto& vertices = meshComp.mesh->getVertices();
                        const auto& indices = meshComp.mesh->getIndices();

                        for (size_t j = 0; j < indices.size(); j += 3)
                        {
                            float tTri;
                            if (intersectRayTriangle(
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
                bool hitLeft = intersectRayAABB(ray, sceneBVH.nodes[leftIdx].bounds, tLeft);
                bool hitRight = intersectRayAABB(ray, sceneBVH.nodes[rightIdx].bounds, tRight);

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
            else if (fieldType->isEnum && fieldJson.isValue())
            {
                int value = fieldJson.get<int>();
                std::memcpy(fieldPtr, &value, sizeof(int));
            }
            if (fieldJson.isValue() && field.loadResource)
            {
                std::string path = fieldJson.get<std::string>();
                field.loadResource(fieldPtr, path); 
            }
            else if (!fieldType->fields.empty() && fieldJson.isObject())
            {
                deserializeFields(fieldJson, fieldPtr, fieldType);
            }
        }
    }
};
