#ifndef SRC_RENDERER_SCENE_HPP
#define SRC_RENDERER_SCENE_HPP

#include "Ecs/ecs.hpp"

#include <Reflection/Reflection.hpp>

#include <Math/Math.hpp>
#include <Math/Matrix/Matrix.hpp>
#include <Math/Vector3.hpp>

#include <Renderer/Mesh.hpp>


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
        Node() noexcept = default;

        Node(
            Ecs::EntityID id,
            Scene* scene
        ) noexcept;

        Ecs::EntityID getId() const noexcept;

        template <typename T>
        void addComponent(const T& component);

        template <typename T>
        T& getComponent();

        template <typename T>
        bool hasComponent() noexcept;

        void setName(std::string_view name);

    private:
        Ecs::EntityID entity = 0;
        Scene* scene = nullptr;

        friend class Scene;
    };

    class Scene
    {
    public:
        static Scene& getInstance() noexcept
        {
            static Scene scene;
            return scene;
        }

        Node createNode(Ecs::EntityID parent = 0) noexcept;

        template <typename T>
        void addComponent(
            Ecs::EntityID entity,
            const T& component
        )
        {
            ecs.add(Ecs::Entity{entity}, component);
        }

        template <typename T>
        T& getComponent(Ecs::EntityID entity)
        {
            return ecs.get<T>(Ecs::Entity{entity});
        }

        template <typename T>
        bool hasComponent(Ecs::EntityID entity)
        {
            return ecs.has<T>(Ecs::Entity{entity});
        }

        Node getNode(Ecs::EntityID id) noexcept;

        Ecs::ECSRegistry& getRegistry() noexcept;

        Ecs::Entity getRootEntity() noexcept;

        void updateWorldTransform(
            Ecs::EntityID entityId,
            const nb::Math::Mat4<float>& parentTransform,
            bool parentDirty = false
        ) noexcept;

        void traverse(
            Ecs::EntityID entityId,
            const std::function<void(Ecs::EntityID)>& action
        ) noexcept;

        void updateAllTransforms() noexcept;

        void traverseAll(const std::function<void(Ecs::EntityID)>& action) noexcept;

    private:
        Scene() noexcept;

        Ecs::ECSRegistry ecs;

        Ecs::EntityID rootEntity;
    };

    template <typename T>
    void Node::addComponent(const T& component)
    {
        scene->addComponent(entity, component);
    }

    template <typename T>
    T& Node::getComponent()
    {
        return scene->getComponent<T>(entity);
    }

    template <typename T>
    inline bool Node::hasComponent() noexcept
    {
        return scene->hasComponent<T>(entity);
    }

}; 

#endif