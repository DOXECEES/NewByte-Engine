#include "Physics.hpp"
#include "Math/Collision/OBB.hpp"
#include <algorithm>
#include <cmath>

namespace nb::Physics
{
    using Mat3 = Math::Mat3<float>;
    using Vec3 = Math::Vector3<float>;

    // ===================================================================
    // Расчёт инвертированного тензора инерции (Мировые координаты)
    // ===================================================================
    Mat3 getInvInertiaWorld(
        const Rigidbody& rb,
        const Math::OBB& obb
    )
    {
        if (rb.isStatic || rb.mass <= 0.0f)
        {
            return Mat3();
        }

        // 1. Локальный инвертированный тензор (диагональ)
        Vec3 s          = obb.halfSize * 2.0f;
        Vec3 invI_local = {
            12.0f / (rb.mass * (s.y * s.y + s.z * s.z)),
            12.0f / (rb.mass * (s.x * s.x + s.z * s.z)), 12.0f / (rb.mass * (s.x * s.x + s.y * s.y))
        };

        // 2. Матрица вращения R (Столбцы = оси OBB)
        // Это гарантирует соответствие физики и рендера
        Mat3 R;
        for (int i = 0; i < 3; ++i)
        {
            R[i][0] = obb.axes[0][i]; // Столбец 0
            R[i][1] = obb.axes[1][i]; // Столбец 1
            R[i][2] = obb.axes[2][i]; // Столбец 2
        }

        // 3. I_inv_world = R * D * R^T
        Mat3 res;
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                res[i][j] = (R[i][0] * invI_local.x * R[j][0]) +
                            (R[i][1] * invI_local.y * R[j][1]) + (R[i][2] * invI_local.z * R[j][2]);
            }
        }
        nb::Error::ErrorManager::instance().report(nb::Error::Type::FATAL, "Det^")
            .with("Value", Math::determinant(res));

        return res;
    }

    void getOBBVertices(
        const Math::OBB& obb,
        Vec3*            v
    )
    {
        Vec3 x = obb.axes[0] * obb.halfSize.x;
        Vec3 y = obb.axes[1] * obb.halfSize.y;
        Vec3 z = obb.axes[2] * obb.halfSize.z;
        v[0]   = obb.center + x + y + z;
        v[1]   = obb.center + x + y - z;
        v[2]   = obb.center + x - y + z;
        v[3]   = obb.center + x - y - z;
        v[4]   = obb.center - x + y + z;
        v[5]   = obb.center - x + y - z;
        v[6]   = obb.center - x - y + z;
        v[7]   = obb.center - x - y - z;
    }

    void PhysicsSystem::applyForce(
        nb::Physics::Rigidbody& rb,
        float                   dt,
        TransformComponent&     t
    ) noexcept
    {
        if (rb.isStatic)
        {
            return;
        }

        // Гравитация
        if (rb.useGravity)
        {
            rb.force += Math::Vector3<float>(0.0f, -std::abs(GRAVITY_ACCELERATION) * rb.mass, 0.0f);
        }

        // Интеграция скорости
        rb.velocity += (rb.force / rb.mass) * dt;
        rb.velocity *= std::pow(std::max(0.0f, 1.0f - rb.drag), dt);
        t.position += rb.velocity * dt;

        if (!rb.freezeRotation)
        {
            rb.angularVelocity *= std::pow(std::max(0.0f, 1.0f - rb.angularDrag), dt);

            if (rb.angularVelocity.squaredLength() > 1e-8f)
            { // ОЧЕНЬ маленький порог
                t.rotation.x += rb.angularVelocity.x * dt;
                t.rotation.y += rb.angularVelocity.y * dt;
                t.rotation.z += rb.angularVelocity.z * dt;
            }
        }

        rb.force  = {0, 0, 0};
        rb.torque = {0, 0, 0};
        t.dirty   = true;
    }

    void PhysicsSystem::resolveDynamicCollisions(
        Scene& scene,
        float  dt
    )
    {
        auto      entities   = scene.getEntitiesWith<Rigidbody, Collider, TransformComponent>();
        const int iterations = 8;
        Vec3      gravity(0.0f, -std::abs(GRAVITY_ACCELERATION), 0.0f);

        for (int step = 0; step < iterations; ++step)
        {
            for (size_t i = 0; i < entities.size(); ++i)
            {
                for (size_t j = i + 1; j < entities.size(); ++j)
                {

                    auto& rbA = scene.getComponent<Rigidbody>(entities[i].id);
                    auto& rbB = scene.getComponent<Rigidbody>(entities[j].id);
                    if (rbA.isStatic && rbB.isStatic)
                    {
                        continue;
                    }

                    auto& tA = scene.getComponent<TransformComponent>(entities[i].id);
                    auto& tB = scene.getComponent<TransformComponent>(entities[j].id);

                    Math::OBB obbA, obbB;
                    obbA.update(tA, scene.getComponent<Collider>(entities[i].id));
                    obbB.update(tB, scene.getComponent<Collider>(entities[j].id));

                    Math::CollisionManifold col = Math::checkOBBCollision(obbA, obbB);
                    if (!col.collided)
                    {
                        continue;
                    }

                    Vec3 normal = col.normal;
                    if (normal.dot(tB.position - tA.position) < 0.0f)
                    {
                        normal = -normal;
                    }

                    // --- ШАГ 1: ГЕНЕРАЦИЯ КОНТАКТОВ С ИНДИВИДУАЛЬНОЙ ГЛУБИНОЙ ---
                    struct Contact
                    {
                        Vec3  point;
                        float depth;
                    };
                    std::vector<Contact> contacts;

                    auto collect = [&](const Math::OBB& incident, const Math::OBB& reference,
                                       const Vec3& n, bool isA)
                    {
                        Vec3 v[8];
                        getOBBVertices(incident, v);
                        float refRadius = reference.getProjectionRadius(n);
                        for (int k = 0; k < 8; ++k)
                        {
                            float distToCenter = (isA ? n : -1.0f * n).dot(v[k] - reference.center);
                            float d            = refRadius - distToCenter;
                            if (d >= -0.01f)
                            {
                                contacts.push_back({v[k], d});
                            }
                        }
                    };
                    collect(obbA, obbB, normal, true);
                    collect(obbB, obbA, normal, false);

                    if (contacts.empty())
                    {
                        contacts.push_back({col.contactPoint, col.depth});
                    }

                    float invMA = rbA.isStatic ? 0.0f : 1.0f / rbA.mass;
                    float invMB = rbB.isStatic ? 0.0f : 1.0f / rbB.mass;
                    Mat3  invIA = getInvInertiaWorld(rbA, obbA);
                    Mat3  invIB = getInvInertiaWorld(rbB, obbB);

                    // --- ШАГ 2: МЯГКОЕ ПОЗИЦИОННОЕ РАЗДЕЛЕНИЕ ---
                    // Мы используем очень маленький коэффициент, чтобы убрать прыжки
                    float percent = 0.1f;
                    float slop    = 0.01f;
                    Vec3  corr    = normal * (std::max(col.depth - slop, 0.0f) /
                                              (invMA + invMB + 1e-6f) * percent);
                    if (!rbA.isStatic)
                    {
                        tA.position -= corr * invMA;
                    }
                    if (!rbB.isStatic)
                    {
                        tB.position += corr * invMB;
                    }

                    // --- ШАГ 3: РАЗРЕШЕНИЕ ИМПУЛЬСОВ ДЛЯ КАЖДОЙ ТОЧКИ ---
                    float cpCount = (float)contacts.size();

                    for (auto& ct : contacts)
                    {
                        Vec3 rA = ct.point - tA.position;
                        Vec3 rB = ct.point - tB.position;

                        // Относительная скорость точки
                        Vec3 rv = (rbB.velocity + Math::cross(rbB.angularVelocity, rB)) -
                                  (rbA.velocity + Math::cross(rbA.angularVelocity, rA));

                        float vDotN = rv.dot(normal);

                        // BIAS: Вычисляется отдельно для каждого угла!
                        // Если один угол глубже другого, он получит более сильный "пинок", что
                        // создаст доворот.
                        float bias = (0.15f / dt) * std::max(0.0f, ct.depth - slop);
                        bias       = std::min(bias, 1.5f); // Ограничиваем, чтобы не прыгало

                        if (vDotN < bias)
                        {
                            Vec3  raN = Math::cross(rA, normal);
                            Vec3  rbN = Math::cross(rB, normal);
                            float ang =
                                (rbA.isStatic ? 0 : Math::cross(invIA * raN, rA).dot(normal)) +
                                (rbB.isStatic ? 0 : Math::cross(invIB * rbN, rB).dot(normal));

                            float j = (-(1.01f) * vDotN + bias) / ((invMA + invMB + ang) * cpCount);
                            j       = std::max(0.0f, j);

                            Vec3 imp = normal * j;
                            if (!rbA.isStatic)
                            {
                                rbA.velocity -= imp * invMA;
                                rbA.angularVelocity -= invIA * Math::cross(rA, imp);
                            }
                            if (!rbB.isStatic)
                            {
                                rbB.velocity += imp * invMB;
                                rbB.angularVelocity += invIB * Math::cross(rB, imp);
                            }

                            rv       = (rbB.velocity + Math::cross(rbB.angularVelocity, rB)) -
                                       (rbA.velocity + Math::cross(rbA.angularVelocity, rA));
                            Vec3 tan = rv - (normal * rv.dot(normal));
                            if (tan.squaredLength() > 1e-6f)
                            {
                                tan      = Math::normalize(tan);
                                float jt = -rv.dot(tan) / ((invMA + invMB + ang) * cpCount);
                                float mu = (rbA.friction + rbB.friction) * 0.5f;
                                float fL = std::abs(j * mu);
                                jt       = std::clamp(jt, -fL, fL);

                                Vec3 fImp = tan * jt;
                                if (!rbA.isStatic)
                                {
                                    rbA.velocity -= fImp * invMA;
                                    rbA.angularVelocity -= invIA * Math::cross(rA, fImp);
                                }
                                if (!rbB.isStatic)
                                {
                                    rbB.velocity += fImp * invMB;
                                    rbB.angularVelocity += invIB * Math::cross(rB, fImp);
                                }
                            }
                        }
                    }

                    if (!rbA.isStatic)
                    {
                        rbA.angularVelocity *= 0.98f;
                    }
                    if (!rbB.isStatic)
                    {
                        rbB.angularVelocity *= 0.98f;
                    }

                    tA.dirty = tB.dirty = true;
                }
            }
        }
    }


} // namespace nb::Physics