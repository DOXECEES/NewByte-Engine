#ifndef SRC_MATH_AABB3D_HPP
#define SRC_MATH_AABB3D_HPP

#include <cstdint>
#include <algorithm>

#include "Vector3.hpp"

namespace nb
{
    namespace Math
    {
        struct AABB3D
        {
            constexpr AABB3D() = default;

            constexpr AABB3D(const float x1, const float y1, const float z1, const float x2, const float y2, const float z2) noexcept
                : minPoint(x1, y1, z1), maxPoint(x2, y2, z2)
            {}
            
            constexpr AABB3D(const nb::Math::Vector3<float>& start, const nb::Math::Vector3<float>& end) noexcept
                : minPoint(start)
                , maxPoint(end)
            {}


            constexpr bool isPointInside(const nb::Math::Vector3<float>& pos)
            {
                return (pos.x >= minPoint.x && pos.y >= minPoint.y && pos.z >= minPoint.z) &&
                        (pos.x <= maxPoint.x && pos.y <= maxPoint.y && pos.z <= maxPoint.z);
            }

            constexpr nb::Math::Vector3<float> center() const noexcept
            {
                return {width() / 2, heigth() / 2, depth() / 2};
            }

            constexpr float width() const noexcept
            {
                return (maxPoint.x + minPoint.x);
            }

            constexpr float heigth() const noexcept
            {
                return (maxPoint.y + minPoint.y);
            }

            constexpr float depth() const noexcept
            {
                return (maxPoint.z + minPoint.z);
            }

            static AABB3D recalculateAabb3dByModelMatrix(const AABB3D& aabb, const Math::Mat4<float> &modelMatrix) noexcept
            {
                Math::Vector3<float> translationVector = {modelMatrix[3][0], modelMatrix[3][1], modelMatrix[3][2]};
    
                Math::Mat3<float> rotate = {
                    {modelMatrix[0][0], modelMatrix[0][1], modelMatrix[0][2]},
                    {modelMatrix[1][0], modelMatrix[1][1], modelMatrix[1][2]},
                    {modelMatrix[2][0], modelMatrix[2][1], modelMatrix[2][2]}
                };
    
                Math::AABB3D B;
    
                B.maxPoint = translationVector;
                B.minPoint = translationVector;
    
                float a;
                float b;
    
    
                for (int i = 0; i < 3; i++)
                {
                    for (int j = 0; j < 3; j++)
                    {
                        a = static_cast<float>((rotate[i][j] * aabb.minPoint[j]));
                        b = static_cast<float>((rotate[i][j] * aabb.maxPoint[j]));
                        if (a < b)
    
                        {
                            B.minPoint[i] += a;
                            B.maxPoint[i] += b;
                        }
                        else
                        {
                            B.minPoint[i] += b;
                            B.maxPoint[i] += a;
                        }
                    }
                }
    
                return B;
            }

            Math::Vector3<float> minPoint = { };
            Math::Vector3<float> maxPoint = { };
        };
    };
};

#endif