#ifndef SRC_MATH_AABB_HPP
#define SRC_MATH_AABB_HPP

#include <cstdint>
#include <algorithm>

#include "Vector2.hpp"

namespace nb
{
    namespace Math
    {
        struct AABB
        {
            constexpr AABB(const float x1, const float y1, const float x2, const float y2) noexcept
                : minPoint(x1, y1), maxPoint(x2, y2)
            {}
            
            constexpr AABB(nb::Math::Vector2 start, nb::Math::Vector2 end) noexcept
                : minPoint(start)
                , maxPoint(end)
            {}

            constexpr bool isPointInside(nb::Math::Vector2 pos) const noexcept  
            {
                return (pos.x >= minPoint.x && pos.y >= minPoint.y) && (pos.x <= maxPoint.x && pos.y <= maxPoint.y);
            }

            constexpr bool isIntersectAABB(AABB oth) const noexcept
            {
                return (this->minPoint.x <= oth.maxPoint.x && this->maxPoint.x >= oth.minPoint.x) &&
                       (this->minPoint.y <= oth.maxPoint.y && this->maxPoint.y >= oth.minPoint.y);
            }

            constexpr AABB merge(const AABB& b) const noexcept
            {
                // Fuck winapi min/max 
                const float x1 = (std::min)(this->minPoint.x, b.minPoint.x);
                const float y1 = (std::min)(this->minPoint.y, b.minPoint.y);
                const float x2 = (std::max)(this->maxPoint.x, b.maxPoint.x);
                const float y2 = (std::max)(this->maxPoint.y, b.maxPoint.y);

                return AABB(x1, y1, x2, y2);
            }

            constexpr void expand(float amount) noexcept
            {
                minPoint.x -= amount;
                minPoint.y -= amount;
                maxPoint.x += amount;
                maxPoint.y += amount;
            }

            constexpr void reduce(float amount) noexcept
            {
                minPoint.x += amount;
                minPoint.y += amount;
                maxPoint.x -= amount;
                maxPoint.y -= amount;
            }

            constexpr float area() const noexcept
            {
                return (width() * height());
            }

            constexpr nb::Math::Vector2 center() const noexcept
            {
                return {width() / 2, height() / 2};
            }

            constexpr float width() const noexcept
            {
                return (maxPoint.x - minPoint.x);
            }

            constexpr float height() const noexcept
            {
                return (maxPoint.y - minPoint.y);
            }


            nb::Math::Vector2 minPoint;
            nb::Math::Vector2 maxPoint;
        };
    };
};

#endif