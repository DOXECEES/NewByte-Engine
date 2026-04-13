#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

#include "Math/Quaternion.hpp"
#include "Math/Vector3.hpp"
#include "Renderer/Scene.hpp"

#include <Reflection/Reflection.hpp>

namespace nb
{
    class Scene;
}

namespace JPH
{
    class BodyInterface;
    class PhysicsSystem;
    class TempAllocatorImpl;
    class JobSystemThreadPool;
} // namespace JPH

namespace nb::Physics
{
    namespace Layers
    {
        static constexpr uint16_t NON_MOVING = 0;
        static constexpr uint16_t MOVING     = 1;
        static constexpr uint16_t NUM_LAYERS = 2;
    } // namespace Layers

    enum class ColliderType
    {
        BOX,
        SPHERE,
        CAPSULE
    };

    struct Collider
    {
        ColliderType type = ColliderType::BOX;

        Math::Vector3<float> halfSize = {0.5f, 0.5f, 0.5f}; 
        float                radius   = 0.5f;               
        float                height   = 1.0f; 

        Math::Vector3<float> offset = {0.0f, 0.0f, 0.0f};

        bool isBox() const noexcept
        {
            return type == ColliderType::BOX;
        }
        bool isSphere() const noexcept
        {
            return type == ColliderType::SPHERE || type == ColliderType::CAPSULE;
        }
        bool isCapsule() const noexcept
        {
            return type == ColliderType::CAPSULE;
        }
    };


    struct Rigidbody
    {
        Rigidbody() noexcept = default;

        JPH::BodyID bodyID;

        float mass        = 1.0f;
        float friction    = 0.4f;
        float restitution = 0.1f;

        bool useGravity     = true;
        bool freezeRotation = false;
        bool isStatic       = false;
        bool isTrigger      = false;

        void addForce(const Math::Vector3<float>& f);

        void addTorque(const Math::Vector3<float>& t);

        void applyImpulse(const Math::Vector3<float>& impulse);
    };

    class PhysicsSystem
    {
    public:
        PhysicsSystem();
        ~PhysicsSystem();

        void init();
        void clear();

        void update(
            Scene& scene,
            float  dt
        );

        void createRigidbody(
            Ecs::EntityID entityId,
            Scene&        scene
        );

        void destroyBody(
            Ecs::EntityID entityId,
            Scene&        scene
        );

        static PhysicsSystem& getInstance();

        JPH::BodyInterface& getBodyInterface();

    private:
        JPH::PhysicsSystem*       physicsSystem = nullptr;
        JPH::TempAllocatorImpl*   tempAllocator = nullptr;
        JPH::JobSystemThreadPool* jobSystem     = nullptr;

        void* bpInterface = nullptr;
        void* objFilter   = nullptr;
        void* bpFilter    = nullptr;

        bool isInitialized = false;
    };
} // namespace nb::Physics

NB_REFLECT_STRUCT(
    nb::Physics::Rigidbody,
    NB_FIELD(
        nb::Physics::Rigidbody,
        mass
    ),
    NB_FIELD(
        nb::Physics::Rigidbody,
        friction
    ),
    NB_FIELD(
        nb::Physics::Rigidbody,
        restitution
    ),
    NB_FIELD(
        nb::Physics::Rigidbody,
        useGravity
    ),
    NB_FIELD(
        nb::Physics::Rigidbody,
        isStatic
    ),
    NB_FIELD(
        nb::Physics::Rigidbody,
        isTrigger
    )
)

NB_REFLECT_ENUM(
    nb::Physics::ColliderType,
    NB_ENUM_VALUE(nb::Physics::ColliderType::BOX),
    NB_ENUM_VALUE(nb::Physics::ColliderType::SPHERE),
    NB_ENUM_VALUE(nb::Physics::ColliderType::CAPSULE)
)

NB_REFLECT_STRUCT(
    nb::Physics::Collider,
    NB_FIELD(
        nb::Physics::Collider,
        type
    ),
    NB_FIELD_VISIBLE_IF(
        nb::Physics::Collider,
        halfSize,
        isBox,
        0.1f
    ),
    NB_FIELD_VISIBLE_IF(
        nb::Physics::Collider,
        radius,
        isSphere,
        0.1f
    ),
    NB_FIELD_VISIBLE_IF(
        nb::Physics::Collider,
        height,
        isCapsule,
        0.1f
    ),
    NB_FIELD(
        nb::Physics::Collider,
        offset
    )
)