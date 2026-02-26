#include "Scene.hpp"

#include <Math/Matrix/Transformation.hpp>


namespace nb
{
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
    }

};
