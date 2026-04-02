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
        static float
        dot(const Math::Vector3<float>& a,
            const Math::Vector3<float>& b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }

        static float length(const Math::Vector3<float>& v)
        {
            return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        }

        static Math::Vector3<float> normalize(const Math::Vector3<float>& v)
        {
            float len = length(v);
            if (len > 0.00001f)
            {
                return {v.x / len, v.y / len, v.z / len};
            }
            return {0, 0, 0};
        }

        static Math::Vector3<float> rotateY(
            const Math::Vector3<float>& v,
            float angleDeg
        )
        {
            float rad = angleDeg * 3.14159265f / 180.0f;
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
    };

    struct Rigidbody
    {
        Math::Vector3<float> velocity{0.0f, 0.0f, 0.0f};
        Math::Vector3<float> acceleration{0.0f, 0.0f, 0.0f};
        Math::Vector3<float> force{0.0f, 0.0f, 0.0f};

        float mass = 1.0f;
        float drag = 0.5f;           
        float friction = 0.4f;       
        float staticFriction = 0.1f; 

        Math::Vector3<float> angularVelocity{0.0f, 0.0f, 0.0f};
        Math::Vector3<float> torque{0.0f, 0.0f, 0.0f};
        float angularDrag = 0.1f;

        bool useGravity = true;
        bool isGrounded = false;
        bool freezeRotation = false;
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
        void update(
            Scene& scene,
            float dt
        )
        {
            if (dt <= 0.0f)
            {
                return;
            }

            for (auto entity : scene.getEntitiesWith<Rigidbody, TransformComponent>())
            {
                auto& rb = scene.getComponent<Rigidbody>(entity.id);
                auto& t = scene.getComponent<TransformComponent>(entity.id);

                rb.isGrounded = false;

                if (rb.useGravity)
                {
                    rb.force += Math::Vector3<float>(0.0f, -9.81f * rb.mass, 0.0f);
                }

                rb.velocity *= (1.0f - rb.drag * dt);

                rb.acceleration = rb.force / rb.mass;
                rb.velocity += rb.acceleration * dt;
                t.position += rb.velocity * dt;

                if (!rb.freezeRotation)
                {
                    rb.angularVelocity *= (1.0f - rb.angularDrag * dt);
                    Math::Vector3<float> angularAcc = rb.torque / rb.mass;
                    rb.angularVelocity += angularAcc * dt;
                    t.rotation += rb.angularVelocity * dt;
                }

                t.dirty = true;
                rb.force = {0, 0, 0};
                rb.torque = {0, 0, 0};
            }

            resolveCollisions(scene, dt);
        }

    private:
        Math::Vector3<float> calculateTerrainNormal(
            const HeightmapCollider& col,
            float x,
            float z
        )
        {
            float eps = 0.1f;
            float hL = col.getHeight(x - eps, z);
            float hR = col.getHeight(x + eps, z);
            float hD = col.getHeight(x, z - eps);
            float hU = col.getHeight(x, z + eps);
            return PhysicsMath::normalize({hL - hR, 2.0f * eps, hD - hU});
        }

        void resolveCollisions(
            Scene& scene,
            float dt
        )
        {
            auto dynamicEntities = scene.getEntitiesWith<Rigidbody, Collider, TransformComponent>();
            auto terrainEntities =
                scene.getEntitiesWith<TerrainColliderComponent, TransformComponent>();

            for (auto dyn : dynamicEntities)
            {
                auto& tDyn = scene.getComponent<TransformComponent>(dyn.id);
                auto& cDyn = scene.getComponent<Collider>(dyn.id);
                auto& rbDyn = scene.getComponent<Rigidbody>(dyn.id);

                for (auto terr : terrainEntities)
                {
                    auto& tTerr = scene.getComponent<TransformComponent>(terr.id);
                    auto& terrComp = scene.getComponent<TerrainColliderComponent>(terr.id);

                    Math::Vector3<float> relPos = tDyn.position - tTerr.position;
                    Math::Vector3<float> localPos = PhysicsMath::rotateY(relPos, -tTerr.rotation.y);

                    float locX = localPos.x / tTerr.scale.x;
                    float locZ = localPos.z / tTerr.scale.z;

                    float locH = terrComp.collider.getHeight(locX, locZ);
                    if (locH < -1e9f)
                    {
                        continue;
                    }

                    float worldGroundY = (locH * tTerr.scale.y) + tTerr.position.y;
                    float feetY = tDyn.position.y - (cDyn.halfSize.y * tDyn.scale.y);

                    if (feetY < worldGroundY)
                    {
                        rbDyn.isGrounded = true;

                        Math::Vector3<float> locNormal =
                            calculateTerrainNormal(terrComp.collider, locX, locZ);
                        Math::Vector3<float> worldNormal =
                            PhysicsMath::rotateY(locNormal, tTerr.rotation.y);

                        tDyn.position.y = worldGroundY + (cDyn.halfSize.y * tDyn.scale.y);
                        tDyn.dirty = true;

                        Math::Vector3<float> gravity(0.0f, -9.81f, 0.0f);
                        float normalPressure = PhysicsMath::dot(gravity, worldNormal);
                        Math::Vector3<float> normalForceVec = worldNormal * normalPressure;
                        Math::Vector3<float> slideForce = gravity - normalForceVec;

                        float speed = PhysicsMath::length(rbDyn.velocity);
                        if (speed > 0.01f)
                        {
                            Math::Vector3<float> velDir = PhysicsMath::normalize(rbDyn.velocity);
                            float frictionMag = std::abs(normalPressure) * rbDyn.friction;
                            rbDyn.force -= velDir * frictionMag;
                        }

                        if (PhysicsMath::length(slideForce) < rbDyn.staticFriction && speed < 0.2f)
                        {
                            rbDyn.velocity = {0, 0, 0};
                        }
                        else
                        {
                            rbDyn.force += slideForce * rbDyn.mass;
                        }

                        float vDotN = PhysicsMath::dot(rbDyn.velocity, worldNormal);
                        if (vDotN < 0)
                        {
                            rbDyn.velocity -= worldNormal * vDotN;
                        }
                    }
                }
            }
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
    )
)

NB_REFLECT_STRUCT(
    nb::Physics::Collider,
    NB_FIELD(
        nb::Physics::Collider,
        halfSize
    )
)

NB_REFLECT_STRUCT(
    nb::Physics::GroundTag
)

NB_REFLECT_STRUCT(
    nb::Physics::TerrainColliderComponent
)