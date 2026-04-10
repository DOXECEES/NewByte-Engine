#pragma once

#include "Math/Vector3.hpp"
#include "Renderer/Scene.hpp"
#include "TerrainCollider.hpp"
#include <Reflection/Reflection.hpp>
#include <algorithm>
#include <cmath>

namespace nb::Physics
{
    struct PhysicsMath
    {
        static Math::Vector3<float> rotateY(
            const Math::Vector3<float>& v,
            float                       angleDeg
        )
        {
            float rad  = Math::toRadians(angleDeg);
            float cosA = std::cos(rad);
            float sinA = std::sin(rad);
            return {v.x * cosA - v.z * sinA, v.y, v.x * sinA + v.z * cosA};
        }
    };

    struct TerrainColliderComponent
    {
        HeightmapCollider collider;
        TerrainColliderComponent() = default;
        TerrainColliderComponent(HeightmapCollider&& col) : collider(std::move(col))
        {
        }

        float bakeStep = 0.1f;
        

    };

    

    struct Rigidbody
    {
        Math::Vector3<float> velocity{0.0f, 0.0f, 0.0f};
        Math::Vector3<float> acceleration{0.0f, 0.0f, 0.0f};
        Math::Vector3<float> force{0.0f, 0.0f, 0.0f};

        float mass           = 1.0f;
        float drag           = 0.5f;
        float friction       = 0.4f;
        float staticFriction = 0.1f;

        Math::Vector3<float> angularVelocity{0.0f, 0.0f, 0.0f};
        Math::Vector3<float> torque{0.0f, 0.0f, 0.0f};
        float                angularDrag = 0.05f;

        bool useGravity     = true;
        bool isGrounded     = false;
        bool freezeRotation = false;
        bool isStatic       = false;
    };

    struct Collider
    {
        Math::Vector3<float> halfSize{0.5f, 0.5f, 0.5f};
    };

    struct GroundTag
    {
    };

    class PhysicsSystem
    {
    public:
        static constexpr float GRAVITY_ACCELERATION     = -9.81f;
        static constexpr float SLEEP_VELOCITY_THRESHOLD = 0.2f;
        static constexpr float MIN_MOVEMENT_THRESHOLD   = 0.01f;
        static constexpr float INVALID_HEIGHT_THRESHOLD = -1e9f;
        static constexpr float TERRAIN_NORMAL_EPS       = 0.1f;

        Math::Vector3<float> calculateInertia(
            float                mass,
            Math::Vector3<float> size
        )
        {
            // Формула для прямоугольного параллелепипеда:
            // I = (1/12) * mass * (other_side1^2 + other_side2^2)
            float x2 = size.x * size.x;
            float y2 = size.y * size.y;
            float z2 = size.z * size.z;

            return {
                (1.0f / 12.0f) * mass * (y2 + z2), (1.0f / 12.0f) * mass * (x2 + z2),
                (1.0f / 12.0f) * mass * (x2 + y2)
            };
        }


        void update(
            Scene& scene,
            float  dt
        )
        {
            if (dt <= 0.0f)
            {
                return;
            }

            for (auto entity : scene.getEntitiesWith<Rigidbody, TransformComponent>())
            {
                auto& rb = scene.getComponent<Rigidbody>(entity.id);
                auto& t  = scene.getComponent<TransformComponent>(entity.id);

                applyForce(rb, dt, t);
            }

            resolveDynamicCollisions(scene, dt);

            resolveCollisions(scene, dt);
        }

        void applyForce(
            nb::Physics::Rigidbody& rb,
            float                   dt,
            TransformComponent&     t
        ) noexcept;
        

    private:
        Math::Vector3<float> calculateTerrainNormal(
            const HeightmapCollider& col,
            float                    x,
            float                    z
        ) noexcept
        {
            float eps = 0.1f;
            float hL  = col.getHeight(x - eps, z);
            float hR  = col.getHeight(x + eps, z);
            float hD  = col.getHeight(x, z - eps);
            float hU  = col.getHeight(x, z + eps);

            return Math::normalize(Math::Vector3<float>{hL - hR, 2.0f * eps, hD - hU});
        }

        void resolveDynamicCollisions(
            Scene& scene,
            float  dt
        );
        

            


        void resolveCollisions(
            Scene& scene,
            float  dt
        )
        {
            auto dynamicEntities = scene.getEntitiesWith<Rigidbody, Collider, TransformComponent>();
            auto terrainEntities =
                scene.getEntitiesWith<TerrainColliderComponent, TransformComponent>();

            for (auto dyn : dynamicEntities)
            {
                auto& tDyn  = scene.getComponent<TransformComponent>(dyn.id);
                auto& cDyn  = scene.getComponent<Collider>(dyn.id);
                auto& rbDyn = scene.getComponent<Rigidbody>(dyn.id);

                for (auto terr : terrainEntities)
                {
                    auto& tTerr    = scene.getComponent<TransformComponent>(terr.id);
                    auto& terrComp = scene.getComponent<TerrainColliderComponent>(terr.id);

                    bool flag = processTerrainCollisions(tDyn, tTerr, terrComp, cDyn, rbDyn);
                    if (!flag)
                    {
                        continue;
                    }
                }
            }
        }

        bool processTerrainCollisions(
            TransformComponent&                    dynamicObjectTransform,
            TransformComponent&                    terrainTransform,
            nb::Physics::TerrainColliderComponent& terrainCollider,
            nb::Physics::Collider&                 dynamicCollider,
            nb::Physics::Rigidbody&                dynamicRigidbody
        )
        {
            Math::Vector3<float> relPos =
                dynamicObjectTransform.position - terrainTransform.position;
            Math::Vector3<float> localPos =
                PhysicsMath::rotateY(relPos, -terrainTransform.rotation.y);

            float locX = localPos.x / terrainTransform.scale.x;
            float locZ = localPos.z / terrainTransform.scale.z;

            float locH = terrainCollider.collider.getHeight(locX, locZ);
            if (!terrainCollider.collider.isValidHeight(locH))
            {

                return false;
            }

            float worldGroundY = (locH * terrainTransform.scale.y) + terrainTransform.position.y;
            float feetY        = dynamicObjectTransform.position.y -
                                 (dynamicCollider.halfSize.y * dynamicObjectTransform.scale.y);

            if (feetY < worldGroundY)
            {
                dynamicRigidbody.isGrounded = true;

                Math::Vector3<float> locNormal =
                    calculateTerrainNormal(terrainCollider.collider, locX, locZ);
                Math::Vector3<float> worldNormal =
                    PhysicsMath::rotateY(locNormal, terrainTransform.rotation.y);

                dynamicObjectTransform.position.y =
                    worldGroundY + (dynamicCollider.halfSize.y * dynamicObjectTransform.scale.y);
                dynamicObjectTransform.dirty = true;

                Math::Vector3<float> gravity(0.0f, GRAVITY_ACCELERATION, 0.0f);
                float                normalPressure = gravity.dot(worldNormal);
                Math::Vector3<float> normalForceVec = worldNormal * normalPressure;
                Math::Vector3<float> slideForce     = gravity - normalForceVec;

                float speed = dynamicRigidbody.velocity.length();
                if (speed > MIN_MOVEMENT_THRESHOLD)
                {
                    Math::Vector3<float> velDir = Math::normalize(dynamicRigidbody.velocity);
                    float frictionMag = std::abs(normalPressure) * dynamicRigidbody.friction;
                    dynamicRigidbody.force -= velDir * frictionMag;
                }

                if (slideForce.length() < dynamicRigidbody.staticFriction &&
                    speed < SLEEP_VELOCITY_THRESHOLD)
                {
                    dynamicRigidbody.velocity = {0, 0, 0};
                }
                else
                {
                    dynamicRigidbody.force += slideForce * dynamicRigidbody.mass;
                }

                float vDotN = dynamicRigidbody.velocity.dot(worldNormal);
                if (vDotN < 0)
                {
                    dynamicRigidbody.velocity -= worldNormal * vDotN;
                }
            }

            return true;
        }
    };
} // namespace nb::Physics

NB_REFLECT_STRUCT(
    nb::Physics::Rigidbody,
    NB_FIELD(
        nb::Physics::Rigidbody,
        velocity
    ),
    NB_FIELD(
        nb::Physics::Rigidbody,
        mass
    ),
    NB_FIELD(
        nb::Physics::Rigidbody,
        drag
    ),
    NB_FIELD(
        nb::Physics::Rigidbody,
        friction
    ),
    NB_FIELD(
        nb::Physics::Rigidbody,
        staticFriction
    ),
    NB_FIELD(
        nb::Physics::Rigidbody,
        angularVelocity
    ),
    NB_FIELD(
        nb::Physics::Rigidbody,
        useGravity
    ),
    NB_FIELD(
        nb::Physics::Rigidbody,
        freezeRotation
    ),
    NB_FIELD(
        nb::Physics::Rigidbody,
        isStatic
    )
)

NB_REFLECT_STRUCT(
    nb::Physics::Collider,
    NB_FIELD(
        nb::Physics::Collider,
        halfSize
    )
)

NB_REFLECT_STRUCT(nb::Physics::GroundTag)



template <>
struct nb::Reflect::ResourceLoader<nb::Physics::HeightmapCollider>
{
    static void load(
        void*              fieldPtr,
        const std::string& path
    )
    {
        [](nb::Physics::HeightmapCollider* field, const std::string& path)
        {
            auto mesh =
                nb::ResMan::ResourceManager::getInstance()->getResource<nb::Renderer::Mesh>(path);
            *field = nb::Physics::bakeMesh(*mesh, 0.1f);
        }(reinterpret_cast<nb::Physics::HeightmapCollider*>(fieldPtr), path);
    }
    static std::string getPath(void* fieldPtr)
    {
        nb::Physics::HeightmapCollider* ptr =
            reinterpret_cast<nb::Physics::HeightmapCollider*>(fieldPtr);
        if (!ptr)
        {
            return "";
        }
        
        return ptr->getPath();
    }
};

NB_REFLECT_STRUCT(
    nb::Physics::TerrainColliderComponent,
    NB_FIELD(
        nb::Physics::TerrainColliderComponent,
        bakeStep
    ),
    NB_FIELD(
        nb::Physics::TerrainColliderComponent,
        collider
    )
)