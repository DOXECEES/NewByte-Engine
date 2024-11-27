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
            
            constexpr AABB3D(nb::Math::Vector3<float> start, nb::Math::Vector3<float> end) noexcept
                : minPoint(start)
                , maxPoint(end)
            {}



            Math::Vector3<float> minPoint;
            Math::Vector3<float> maxPoint;
        };
    };
};

#endif