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



            Math::Vector3<float> minPoint = { };
            Math::Vector3<float> maxPoint = { };
        };
    };
};

#endif