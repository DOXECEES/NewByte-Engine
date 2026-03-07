#ifndef SRC_RENDERER_SCENE_HPP
#define SRC_RENDERER_SCENE_HPP

#include "Ecs/ecs.hpp"

#include <Reflection/Reflection.hpp>

#include <Math/Math.hpp>
#include <Math/Matrix/Matrix.hpp>
#include <Math/Vector3.hpp>

#include <Renderer/Mesh.hpp>

#include <algorithm>
#undef min
#undef max

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

        struct PickingResult
        {
            Ecs::EntityID entityId = 0;
            //float distance = std::numeric_limits<float>::max();
        };

        Ecs::EntityID pickNode(const Math::Ray& ray)
        {
            Ecs::EntityID selectedId = 0;
            float closestDist = std::numeric_limits<float>::max();

            traverseAll(
                [&](Ecs::EntityID id)
                {
                    if (!hasComponent<MeshComponent>(id) || !hasComponent<TransformComponent>(id))
                    {
                        return;
                    }

                    auto& transform = getComponent<TransformComponent>(id);
                    auto& meshComp = getComponent<MeshComponent>(id);

                    // 1. Берем локальный AABB меша (рассчитанный при загрузке модели)
                    Math::AABB3D localAABB = meshComp.mesh->getAabb3d();

                    // 2. Генерируем 8 углов локального бокса
                    Math::Vector3<float> corners[8] = {
                        {localAABB.minPoint.x, localAABB.minPoint.y, localAABB.minPoint.z},
                        {localAABB.maxPoint.x, localAABB.minPoint.y, localAABB.minPoint.z},
                        {localAABB.minPoint.x, localAABB.maxPoint.y, localAABB.minPoint.z},
                        {localAABB.maxPoint.x, localAABB.maxPoint.y, localAABB.minPoint.z},
                        {localAABB.minPoint.x, localAABB.minPoint.y, localAABB.maxPoint.z},
                        {localAABB.maxPoint.x, localAABB.minPoint.y, localAABB.maxPoint.z},
                        {localAABB.minPoint.x, localAABB.maxPoint.y, localAABB.maxPoint.z},
                        {localAABB.maxPoint.x, localAABB.maxPoint.y, localAABB.maxPoint.z}
                    };

                    // 3. Находим границы мирового AABB, трансформируя каждый угол
                    Math::Vector3<float> worldMin(std::numeric_limits<float>::max());
                    Math::Vector3<float> worldMax(-std::numeric_limits<float>::max());

                    for (int i = 0; i < 8; ++i)
                    {
                        auto& m = transform.worldMatrix;
                        auto& c = corners[i];

                        // Трансформируем точку через мировую матрицу
                        // Учитывая ваш Row-Major оператор*: Vector4(v, 1.0) * worldMatrix
                        Math::Vector4<float> worldCorner4(
                            c.x * m[0][0] + c.y * m[1][0] + c.z * m[2][0] + m[3][0],
                            c.x * m[0][1] + c.y * m[1][1] + c.z * m[2][1] + m[3][1],
                            c.x * m[0][2] + c.y * m[1][2] + c.z * m[2][2] + m[3][2],
                            1.0f // w компонент
                        );


                        // Если у вас Column-Major: worldMatrix * Vector4(corners[i], 1.0f)
                        // Но судя по вашему inverse, у вас Row-Major.

                        worldMin.x = std::min(worldMin.x, worldCorner4.x);
                        worldMin.y = std::min(worldMin.y, worldCorner4.y);
                        worldMin.z = std::min(worldMin.z, worldCorner4.z);

                        worldMax.x = std::max(worldMax.x, worldCorner4.x);
                        worldMax.y = std::max(worldMax.y, worldCorner4.y);
                        worldMax.z = std::max(worldMax.z, worldCorner4.z);
                    }

                    Math::AABB3D worldAABB = {worldMin, worldMax};

                    // 4. Проверка пересечения луча с мировым боксом
                    float t = 0.0f;
                    if (intersectRayAABB(ray, worldAABB, t))
                    {
                        if (t > 0.0f && t < closestDist)
                        {
                            closestDist = t;
                            selectedId = id;
                        }
                    }
                }
            );

            return selectedId;
        }

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