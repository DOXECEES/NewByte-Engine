#include "Physics.hpp"
#include "Math/Collision/OBB.hpp"
#include <algorithm>
#include <cmath>

namespace nb::Physics
{
    using Mat3 = Math::Mat3<float>;
    using Vec3 = Math::Vector3<float>;
    using Quat = Math::Quaternion<float>;



    Mat3 getInvInertiaWorld(
        const Rigidbody&          rb,
        const TransformComponent& t,
        const Math::OBB&          obb
    )
    {
        if (rb.isStatic || rb.mass <= 0.0f)
        {
            return Mat3();
        }

        Vec3  s          = obb.halfSize * 2.0f;
        float m          = rb.mass;
        Vec3  invI_local = {
            12.0f / (m * (s.y * s.y + s.z * s.z)), 12.0f / (m * (s.x * s.x + s.z * s.z)),
            12.0f / (m * (s.x * s.x + s.y * s.y))
        };

        Mat3 res;
        for (int i = 0; i < 3; ++i)
        {
            const Vec3& axis     = obb.axes[i];
            float       localInv = (i == 0) ? invI_local.x : (i == 1 ? invI_local.y : invI_local.z);

            for (int row = 0; row < 3; ++row)
            {
                for (int col = 0; col < 3; ++col)
                {
                    res[row][col] += localInv * axis[row] * axis[col];
                }
            }
        }
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
        Rigidbody&          rb,
        float               dt,
        TransformComponent& t,
        Collider&           c
    ) noexcept
    {
        if (rb.isStatic)
        {
            return;
        }

        if (rb.useGravity)
        {
            rb.force += Vec3(0.0f, -9.81f * rb.mass, 0.0f);
        }
        rb.velocity += (rb.force / rb.mass) * dt;
        rb.velocity *= std::pow(std::max(0.0f, 1.0f - rb.drag), dt);
        t.position += rb.velocity * dt;

        if (!rb.freezeRotation)
        {
            Math::OBB tempObb;
            tempObb.update(t, c);
            Mat3 invI = getInvInertiaWorld(rb, t, tempObb);

            rb.angularVelocity += (invI * rb.torque) * dt;
            rb.angularVelocity *= std::pow(std::max(0.0f, 1.0f - rb.angularDrag), dt);

            if (rb.angularVelocity.squaredLength() > 1e-9f)
            {
                Quat omega(rb.angularVelocity.x, rb.angularVelocity.y, rb.angularVelocity.z, 0.0f);
                t.rotation = t.rotation - (omega * t.rotation) * (0.5f * dt);
                t.rotation.normalize();
                t.eulerAngle = t.rotation.toEulerXYZ();
                t.lastEuler  = t.eulerAngle;
            }
        }

        rb.force  = {0, 0, 0};
        rb.torque = {0, 0, 0};
        t.dirty   = true;

        if (rb.velocity.squaredLength() < 0.0005f && rb.angularVelocity.squaredLength() < 0.0005f)
        {
            rb.velocity        = {0, 0, 0};
            rb.angularVelocity = {0, 0, 0};
        }
    }

    void PhysicsSystem::resolveDynamicCollisions(
        Scene& scene,
        float  dt
    )
    {
        auto        entities   = scene.getEntitiesWith<Rigidbody, Collider, TransformComponent>();
        const int   iterations = 10;
        const float slop       = 0.01f;
        const float baumgarte  = 0.1f; 

        for (int step = 0; step < iterations; ++step)
        {
            for (size_t i = 0; i < entities.size(); ++i)
            {
                for (size_t k = i + 1; k < entities.size(); ++k)
                {
                    auto& rbA = scene.getComponent<Rigidbody>(entities[i].id);
                    auto& rbB = scene.getComponent<Rigidbody>(entities[k].id);
                    if (rbA.isStatic && rbB.isStatic)
                    {
                        continue;
                    }

                    auto& tA = scene.getComponent<TransformComponent>(entities[i].id);
                    auto& tB = scene.getComponent<TransformComponent>(entities[k].id);

                    Math::OBB obbA, obbB;
                    obbA.update(tA, scene.getComponent<Collider>(entities[i].id));
                    obbB.update(tB, scene.getComponent<Collider>(entities[k].id));

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

                    float invMA        = rbA.isStatic ? 0.0f : 1.0f / rbA.mass;
                    float invMB        = rbB.isStatic ? 0.0f : 1.0f / rbB.mass;
                    float totalInvMass = invMA + invMB;

                    if (col.depth > slop)
                    {
                        Vec3 corr = normal * ((col.depth - slop) * (baumgarte / iterations) /
                                              (totalInvMass + 1e-6f));
                        if (!rbA.isStatic)
                        {
                            tA.position -= corr * invMA;
                        }
                        if (!rbB.isStatic)
                        {
                            tB.position += corr * invMB;
                        }
                    }

                    struct ContactPoint
                    {
                        Vec3  pos;
                        float depth;
                    };
                    std::vector<ContactPoint> contacts;
                    auto                      collect =
                        [&](const Math::OBB& inc, const Math::OBB& ref, const Vec3& n, bool isA)
                    {
                        Vec3 v[8];
                        getOBBVertices(inc, v);
                        float refRadius = ref.getProjectionRadius(n);
                        for (int m = 0; m < 8; ++m)
                        {
                            float d = refRadius - (isA ? n : -1.0f * n).dot(v[m] - ref.center);
                            if (d >= -0.01f)
                            {
                                contacts.push_back({v[m], d});
                            }
                        }
                    };
                    collect(obbA, obbB, normal, true);
                    collect(obbB, obbA, normal, false);
                    if (contacts.empty())
                    {
                        contacts.push_back({col.contactPoint, col.depth});
                    }

                    Mat3  invIA   = getInvInertiaWorld(rbA, tA, obbA);
                    Mat3  invIB   = getInvInertiaWorld(rbB, tB, obbB);
                    float cpCount = (float)contacts.size();

                    for (auto& cp : contacts)
                    {
                        Vec3 rA = cp.pos - tA.position;
                        Vec3 rB = cp.pos - tB.position;

                        Vec3 vA = rbA.velocity + Math::cross(rbA.angularVelocity, rA);
                        Vec3 vB = rbB.velocity + Math::cross(rbB.angularVelocity, rB);
                        Vec3 rv = vB - vA;

                        float vDotN = rv.dot(normal);
                        if (vDotN > 0.0f)
                        {
                            continue;
                        }

                        float restitution = (rbA.restitution + rbB.restitution) * 0.5f;
                        if (std::abs(vDotN) < 0.5f)
                        {
                            restitution = 0.0f;
                        }

                        Vec3 raN = Math::cross(rA, normal);
                        Vec3 rbN = Math::cross(rB, normal);

                        float angA = rbA.isStatic ? 0.0f : Math::cross(invIA * raN, rA).dot(normal);
                        float angB = rbB.isStatic ? 0.0f : Math::cross(invIB * rbN, rB).dot(normal);

                        float impulseMag = -(1.0f + restitution) * vDotN;
                        impulseMag /= (totalInvMass + angA + angB + 1e-6f);
                        impulseMag /= cpCount;

                        Vec3 impulse = normal * impulseMag;

                        if (!rbA.isStatic)
                        {
                            rbA.velocity -= impulse * invMA;
                            rbA.angularVelocity -= invIA * Math::cross(rA, impulse);
                        }
                        if (!rbB.isStatic)
                        {
                            rbB.velocity += impulse * invMB;
                            rbB.angularVelocity += invIB * Math::cross(rB, impulse);
                        }

                        vA           = rbA.velocity + Math::cross(rbA.angularVelocity, rA);
                        vB           = rbB.velocity + Math::cross(rbB.angularVelocity, rB);
                        rv           = vB - vA;
                        Vec3 tangent = rv - (normal * rv.dot(normal));
                        if (tangent.squaredLength() > 1e-6f)
                        {
                            tangent.normalize();
                            float inertiaTanA =
                                rbA.isStatic ? 0.0f
                                             : Math::cross(invIA * Math::cross(rA, tangent), rA)
                                                   .dot(tangent);
                            float inertiaTanB =
                                rbB.isStatic ? 0.0f
                                             : Math::cross(invIB * Math::cross(rB, tangent), rB)
                                                   .dot(tangent);
                            float jt =
                                -rv.dot(tangent) /
                                ((totalInvMass + inertiaTanA + inertiaTanB + 1e-6f) * cpCount);
                            float mu = (rbA.friction + rbB.friction) * 0.5f;
                            jt       = std::clamp(jt, -impulseMag * mu, impulseMag * mu);

                            Vec3 fImp = tangent * jt;
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
                    tA.dirty = tB.dirty = true;
                }
            }
        }
    }


} // namespace nb::Physics