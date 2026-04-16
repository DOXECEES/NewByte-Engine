#ifndef SRC_RENDERER_SCENE_HPP
#define SRC_RENDERER_SCENE_HPP

#include "Ecs/ecs.hpp"
#include "Renderer/RendererStructures.hpp"
#include "Serialize/ISerializable.hpp"

#include <Reflection/Reflection.hpp>
#include "Loaders/JSON/Node.hpp"
#include <Math/Math.hpp>
#include <Math/Matrix/Matrix.hpp>
#include <Math/Vector3.hpp>
#include <Math/Quaternion.hpp>
#include "Physics/Physics.hpp"

#include <Renderer/Mesh.hpp>
#include <Reflection/Reflection.hpp>
#include "Renderer/Camera.hpp"

#include <Manager/ResourceManager.hpp>
#include <Resources/MaterialAsset.hpp>
#include "BVH.hpp"
#include "Resources/ResourceDispatcher.hpp"
#include <algorithm>
#undef min
#undef max

struct TransformComponent
{
    nb::Math::Vector3<float> position{};
    nb::Math::Vector3<float> eulerAngle{};
    nb::Math::Vector3<float> lastEuler{}; 

    nb::Math::Quaternion<float> rotation;
     
    nb::Math::Vector3<float> scale{1.0f, 1.0f, 1.0f};
    nb::Math::Mat4<float> localMatrix = nb::Math::Mat4<float>::identity();
    nb::Math::Mat4<float> worldMatrix = nb::Math::Mat4<float>::identity();

    bool dirty = true;
    bool physicsDirty = true;
};

NB_REFLECT_STRUCT(TransformComponent,
    NB_FIELD(TransformComponent, position),
    NB_FIELD(TransformComponent, eulerAngle),
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
NB_REFLECT_INTERNAL_STRUCT(NameComponent,
    NB_FIELD(
        NameComponent,
        name
    )

)


//

struct MeshComponent
{
    std::shared_ptr<nb::Renderer::Mesh> mesh;
    std::vector<Ref<nb::Resource::MaterialAsset>> material;
};

NB_REFLECT_PTR(
    std::shared_ptr<nb::Renderer::Mesh>,
    "std::shared_ptr<nb::Renderer::Mesh>"
)
NB_REFLECT_PTR(
    std::vector<Ref<nb::Resource::MaterialAsset>>, 
    "std::vector<Ref<nb::Resource::MaterialAsset>>"
)

NB_REFLECT_RESOURCE_PTR(
    std::shared_ptr<nb::Renderer::Mesh>,
    "MeshPtr",
    [](std::shared_ptr<nb::Renderer::Mesh>* field,
       const std::string& path)
    {
        *field = nb::ResMan::ResourceManager::getInstance()->getResource<nb::Renderer::Mesh>(path);
    }
)

NB_REFLECT_RESOURCE_VECTOR_PTR(
    std::vector<Ref<nb::Resource::MaterialAsset>>,
    nb::Resource::MaterialAsset,
    "MaterialVector",
    [](std::vector<Ref<nb::Resource::MaterialAsset>>* field,
       const std::vector<std::string>&                paths)
    {
        field->clear();
        for (const auto& path : paths)
        {
            auto res = nb::ResMan::ResourceManager::getInstance()->getResource<nb::Resource::MaterialAsset>(path);
            if (res)
            {
                field->push_back(res);
            }
        }
    }
)


NB_REFLECT_STRUCT(
    MeshComponent,
    NB_FIELD(
        MeshComponent,
        mesh
    ),
    NB_FIELD(
        MeshComponent,
        material
    )
)

struct CameraComponent
{
    std::unique_ptr<nb::Renderer::Camera> controller;
    bool                                  isPrimary = true;

    float fov    = 60.0f;
    float aspect = 1.77f;

    CameraComponent() : controller(std::make_unique<nb::Renderer::Camera>())
    {
    }

    NB_MOVE_ONLY(CameraComponent);
};

NB_REFLECT_STRUCT(
    CameraComponent,
    NB_FIELD(
        CameraComponent,
        isPrimary
    )
)




namespace nb
{
    class Scene;

    class Node
    {

        static constexpr uint32_t INVALID_VALUE = ~uint32_t(0);

    public:
        Node() noexcept;

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

        bool isValid() const noexcept;

        static Node createInvalid() noexcept;

    private:
        Ecs::EntityID entity = 0;
        Scene* scene = nullptr;

        friend class Scene;
    };

    class Scene : public nb::Serialize::ISerializable
    {
    public:
        static Scene& getInstance() noexcept
        {
            static Scene scene;
            return scene;
        }

        void clear() noexcept;
        

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

        template <typename... Args>
        std::vector<Ecs::Entity> getEntitiesWith()
        {
            std::vector<Ecs::Entity> result;

            traverse(
                rootEntity,
                [this, &result](Ecs::EntityID id)
                {
                    if ((ecs.has<Args>({id}) && ...))
                    {
                        result.push_back(Ecs::Entity{id});
                    }
                }
            );

            return result;
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

        Ecs::EntityID pickNode(const Math::Ray& ray) noexcept;
        
        void updateBvh() noexcept;

        void invalidateBvh() noexcept;

        Math::BVH* getBvh() noexcept;


        void serialize(nb::Serialize::IArchive* archive) noexcept override;
        void deserialize(nb::Serialize::IArchive* archive) noexcept override;
        void serializeFields(
            nb::Serialize::IArchive* archive,
            void* object,
            nb::Reflect::TypeInfo* type
        ) noexcept;

        void serializeComponents(
            nb::Serialize::IArchive* archive,
            Ecs::Entity entity
        ) noexcept;

        void deserializeFields(
            const nb::Loaders::Node& node,
            void* object,
            nb::Reflect::TypeInfo* type
        ) noexcept;

        void deserializeComponents(
            const nb::Loaders::Node& node,
            Ecs::Entity entity
        ) noexcept;

        void addComponentRaw(
            Ecs::Entity entity,
            Ecs::StorageWrapperBase* storage,
            void* component
        ) noexcept;
    
        bool isPaused = true;
    
    private:
        Scene() noexcept;

        Ecs::ECSRegistry ecs;

        Ecs::EntityID rootEntity;

        Resources::ResourceDispatcher dispatcher;

        Math::BVH sceneBVH;
        bool bvhDirty = true;
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