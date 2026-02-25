#pragma once

#include "Ecs/ecs.hpp"
#include <string_view>

#include <Reflection/Reflection.hpp>


//
#include <Math/Vector3.hpp>
#include <Math/Matrix/Matrix.hpp>
#include <Math/Math.hpp>
#include <Math/Matrix/Transformation.hpp>


struct TransformComponent
{
    nb::Math::Vector3<float> position{};
    nb::Math::Vector3<float> rotation{};
    nb::Math::Vector3<float> scale{1.0f, 1.0f, 1.0f};
    nb::Math::Mat4<float> localMatrix = nb::Math::Mat4<float>::identity();
    nb::Math::Mat4<float> worldMatrix = nb::Math::Mat4<float>::identity();

    bool dirty = true;
};



NB_REFLECT_STRUCT(TransformComponent,
    NB_FIELD(TransformComponent, position),
    NB_FIELD(TransformComponent, rotation),
    NB_FIELD(TransformComponent, scale),
    NB_FIELD(TransformComponent, dirty)
)


struct HierarchyComponent
{
    nb::Ecs::EntityID parent = 0;
    std::vector<nb::Ecs::EntityID> children;
};

NB_REFLECT_INTERNAL_STRUCT(HierarchyComponent)

struct NameComponent
{
    std::string name;
};
NB_REFLECT_INTERNAL_STRUCT(NameComponent)

//

struct MeshComponent
{
    std::shared_ptr<nb::Renderer::Mesh> mesh;
};
NB_REFLECT_STRUCT(MeshComponent)


namespace nb
{

    class Scene;

    class Node
    {
    public:
        Node() = default;

        Node(
            Ecs::EntityID id,
            Scene* scene
        )
            : entity(id),
              scene(scene)
        {
        }

        Ecs::EntityID getId() const noexcept
        {
            return entity;
        }

        template <typename T> void addComponent(const T& component);

        template <typename T> T& getComponent();
        template <typename T> bool hasComponent() noexcept;

        void setName(std::string_view name);

    private:
        Ecs::EntityID entity = 0;
        Scene* scene = nullptr;

        friend class Scene;
    };

    /* =========================================================
     * Scene
     * =========================================================*/

    class Scene
    {
    public:

        static Scene& getInstance() noexcept
        {
            static Scene scene;
            return scene;
        }

        Node createNode(Ecs::EntityID parent = 0)
        {
            auto entity = ecs.createEntity();

            // Root node fallback
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

        template <typename T>
        void addComponent(
            Ecs::EntityID entity,
            const T& component
        )
        {
            ecs.add(Ecs::Entity{entity}, component);
        }

        template <typename T> T& getComponent(Ecs::EntityID entity)
        {
            return ecs.get<T>(Ecs::Entity{entity});
        }

        template <typename T>
        bool hasComponent(Ecs::EntityID entity)
        {
            return ecs.has<T>(Ecs::Entity{entity});
        }

        Node getNode(Ecs::EntityID id)
        {
            return Node{id, this};
        }

        Ecs::ECSRegistry& getRegistry() noexcept
        {
            return ecs;
        }

        Ecs::Entity getRootEntity() noexcept
        {
            return Ecs::Entity{rootEntity};
        }


        void updateWorldTransform(
            Ecs::EntityID entityId,
            const nb::Math::Mat4<float>& parentTransform,
            bool parentDirty = false
        )
        {
            auto& transform = ecs.get<TransformComponent>(Ecs::Entity{entityId});
            auto& hierarchy = ecs.get<HierarchyComponent>(Ecs::Entity{entityId});

            // Объект грязный, если он сам изменился ИЛИ изменился его родитель
            bool isDirty = transform.dirty || parentDirty;

            if (isDirty)
            {
                // 1. Начинаем с чистой единичной матрицы
                nb::Math::Mat4<float> local = nb::Math::Mat4<float>::identity();

                // 2. Собираем матрицу в правильном порядке: Translation * Rotation * Scale
                // ВНИМАНИЕ: Порядок вызовов зависит от того, как ваша библиотека перемножает
                // матрицы. Обычно для классического TRS вызовы идут так:
                local = Math::scale(local, transform.scale);


                // Вращение (порядок Euler углов обычно YXZ или XYZ)
                local = Math::rotate(local, {1.0f, 0.0f, 0.0f}, transform.rotation.x);
                local = Math::rotate(
                    local, {0.0f, 1.0f, 0.0f}, transform.rotation.y
                ); // Исправлено на y
                local = Math::rotate(local, {0.0f, 0.0f, 1.0f}, transform.rotation.z);
                local = Math::translate(local, transform.position);


                transform.localMatrix = local;

                // 3. Вычисляем мировую матрицу: МирРодителя * ЛокальнаяТекущая
                transform.worldMatrix = parentTransform * transform.localMatrix;

                // Сбрасываем флаг, так как мы обновили данные
                transform.dirty = false;
            }

            // Рекурсивно обновляем детей, передавая статус "грязности"
            for (auto childId : hierarchy.children)
            {
                updateWorldTransform(childId, transform.worldMatrix, isDirty);
            }
        }

        // Универсальный обход графа
        void traverse(
            Ecs::EntityID entityId,
            const std::function<void(Ecs::EntityID)>& action
        )
        {
            action(entityId);
            auto& hierarchy = ecs.get<HierarchyComponent>(Ecs::Entity{entityId});
            for (auto childId : hierarchy.children)
            {
                traverse(childId, action);
            }
        }

        // Вспомогательный запуск от корня
        void updateAllTransforms()
        {
            updateWorldTransform(rootEntity, nb::Math::Mat4<float>::identity(), false);
        }

        void traverseAll(const std::function<void(Ecs::EntityID)>& action)
        {
            traverse(rootEntity, action);
        }

    private:

        Scene()
        {
            auto root = ecs.createEntity();
            rootEntity = root.id;

            ecs.add(root, TransformComponent{});
            ecs.add(root, HierarchyComponent{0});
            ecs.add(root, NameComponent{"Root"});
        }
        

        Ecs::ECSRegistry ecs;

        Ecs::EntityID rootEntity;
    };

    /* =========================================================
     * Node template forwarding
     * =========================================================*/

    template <typename T> void Node::addComponent(const T& component)
    {
        scene->addComponent(entity, component);
    }

    template <typename T> T& Node::getComponent()
    {
        return scene->getComponent<T>(entity);
    }

    template <typename T> inline bool Node::hasComponent() noexcept
    {
        return scene->hasComponent<T>(entity);
    }

    inline void Node::setName(std::string_view name)
    {
        scene->addComponent(entity, NameComponent{std::string(name)});
    }

} // namespace nb