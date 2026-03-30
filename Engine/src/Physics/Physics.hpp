#pragma once

#include "Math/Vector3.hpp"

#include "Renderer/Scene.hpp"

#include <Reflection/Reflection.hpp>

namespace nb
{
    struct Rigidbody
    {
        Math::Vector3<float> velocity{0.0f, 0.0f, 0.0f};
        Math::Vector3<float> acceleration{0.0f, 0.0f, 0.0f};
        Math::Vector3<float> force{0.0f, 0.0f, 0.0f};

        float mass = 1.0f;
        bool useGravity = true;
    };

    NB_REFLECT_STRUCT(
        Rigidbody,
        NB_FIELD(
            Rigidbody,
            velocity
        ),
        NB_FIELD(
            Rigidbody,
            acceleration
        ),
        NB_FIELD(
            Rigidbody,
            force
        ),
        NB_FIELD(
            Rigidbody,
            mass
        ),
        NB_FIELD(
            Rigidbody,
            useGravity
        )
    )


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
            // 1. Применяем силы и интегрируем движение
            for (auto entity : scene.getEntitiesWith<Rigidbody, TransformComponent>())
            {
                auto& rb = scene.getComponent<Rigidbody>(entity.id);
                auto& t = scene.getComponent<TransformComponent>(entity.id);

                // Добавляем гравитацию как силу
                if (rb.useGravity)
                {
                    rb.force += Math::Vector3<float>(0.0f, -9.81f * rb.mass, 0.0f);
                }

                // Вычисляем ускорение
                rb.acceleration = rb.force / rb.mass;

                // Интегрируем скорость и позицию
                rb.velocity += rb.acceleration * dt;
                t.position += rb.velocity * dt;
                t.dirty = true;

                // Сбрасываем силы на следующий кадр
                rb.force = Math::Vector3<float>(0.0f, 0.0f, 0.0f);
            }

            // 2. Разрешение коллизий
            resolveCollisions(scene);
        }

    private:
        void resolveCollisions(Scene& scene)
        {
            auto dynamicEntities = scene.getEntitiesWith<Rigidbody, Collider, TransformComponent>();
            auto staticEntities = scene.getEntitiesWith<Collider, TransformComponent, GroundTag>();

            for (auto dyn : dynamicEntities)
            {
                auto& tDyn = scene.getComponent<TransformComponent>(dyn.id);
                auto& cDyn = scene.getComponent<Collider>(dyn.id);
                auto& rbDyn = scene.getComponent<Rigidbody>(dyn.id);

                for (auto stat : staticEntities)
                {
                    auto& tStat = scene.getComponent<TransformComponent>(stat.id);
                    auto& cStat = scene.getComponent<Collider>(stat.id);

                    if (checkAABB(tDyn.position, cDyn.halfSize, tStat.position, cStat.halfSize))
                    {
                        // Простое разрешение по оси Y
                        if (rbDyn.velocity.y < 0) 
                        {
                            tDyn.position.y = tStat.position.y + cStat.halfSize.y + cDyn.halfSize.y;
                            rbDyn.velocity.y = 0; 
                            tDyn.dirty = true;
                        }
                    }
                }
            }
        }

        bool checkAABB(
            const Math::Vector3<float>& pos1,
            const Math::Vector3<float>& half1,
            const Math::Vector3<float>& pos2,
            const Math::Vector3<float>& half2
        )
        {
            return (std::abs(pos1.x - pos2.x) <= (half1.x + half2.x)) &&
                   (std::abs(pos1.y - pos2.y) <= (half1.y + half2.y)) &&
                   (std::abs(pos1.z - pos2.z) <= (half1.z + half2.z));
        }
    };

};
