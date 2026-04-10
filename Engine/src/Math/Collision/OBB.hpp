#ifndef SRC_MATH_COLLISON_OBB_HPP
#define SRC_MATH_COLLISON_OBB_HPP

#include "Math/Quaternion.hpp" // Добавили кватернионы
#include "Math/Vector3.hpp"
#include <algorithm>
#include <cmath>

namespace nb::Math
{
    struct OBB
    {
        Math::Vector3<float> center;
        Math::Vector3<float> axes[3];
        Math::Vector3<float> halfSize;

        void update(
            const TransformComponent& t,
            const Physics::Collider&  c
        )
        {
            center   = t.position;
            halfSize = c.halfSize * t.scale;

            auto R = t.rotation.toMatrix3();


            axes[0] = {R[0][0], R[0][1], R[0][2]}; 
            axes[1] = {R[1][0], R[1][1], R[1][2]}; 
            axes[2] = {R[2][0], R[2][1], R[2][2]}; 

            for (int i = 0; i < 3; ++i)
            {
                axes[i].normalize();
            }
        }

        float getProjectionRadius(const Math::Vector3<float>& axis) const
        {
            return std::abs(axes[0].dot(axis) * halfSize.x) +
                   std::abs(axes[1].dot(axis) * halfSize.y) +
                   std::abs(axes[2].dot(axis) * halfSize.z);
        }

        Math::Vector3<float> getSupportPoint(const Math::Vector3<float>& direction) const
        {
            Math::Vector3<float> result = center;
            for (int i = 0; i < 3; ++i)
            {
                if (axes[i].dot(direction) > 0)
                {
                    result += axes[i] * halfSize[i];
                }
                else
                {
                    result -= axes[i] * halfSize[i];
                }
            }
            return result;
        }
    };

    struct CollisionManifold
    {
        Math::Vector3<float> normal;
        Math::Vector3<float> contactPoint;
        float                depth    = 0;
        bool                 collided = false;
    };

    inline CollisionManifold checkOBBCollision(
        const OBB& a,
        const OBB& b
    )
    {
        CollisionManifold result;
        result.collided = false;
        result.depth    = 1e9f;

        Math::Vector3<float> testAxes[15];
        for (int i = 0; i < 3; ++i)
        {
            testAxes[i] = a.axes[i];
        }
        for (int i = 0; i < 3; ++i)
        {
            testAxes[i + 3] = b.axes[i];
        }

        int curr = 6;
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                Math::Vector3<float> cross = Math::cross(a.axes[i], b.axes[j]);
                if (cross.squaredLength() < 0.001f)
                {
                    testAxes[curr++] = {0, 0, 0};
                }
                else
                {
                    testAxes[curr++] = Math::normalize(cross);
                }
            }
        }

        for (const auto& axis : testAxes)
        {
            if (axis.squaredLength() < 0.5f)
            {
                continue;
            }

            float radiusA = a.getProjectionRadius(axis);
            float radiusB = b.getProjectionRadius(axis);
            float dist    = std::abs((b.center - a.center).dot(axis));
            float overlap = (radiusA + radiusB) - dist;

            if (overlap <= 0.0f)
            {
                return result;
            }

            if (overlap < result.depth)
            {
                result.depth  = overlap;
                result.normal = axis;
            }
        }

        if ((b.center - a.center).dot(result.normal) < 0)
        {
            result.normal *= -1.0f;
        }

        result.collided     = true;
        result.contactPoint = a.getSupportPoint(result.normal);
        return result;
    }
} // namespace nb::Math

#endif