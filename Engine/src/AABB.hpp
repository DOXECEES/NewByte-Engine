#pragma once

#include <Windows.h>

#include <cstdint>

struct NBPoint
{
    explicit NBPoint(const int32_t _x, const int32_t _y)
        : x(_x), y(_y)
    {
    }

    int32_t x = 0;
    int32_t y = 0;
};

struct AABB
{
    explicit AABB(const int32_t x, const int32_t y, const int32_t width, const int32_t height)
        : minPoint(x, y), maxPoint(x + width, y + height)
    {
    }

    /*explicit AABB(const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2 )
        : minPoint(x1, y1)
        , maxPoint(x2, y2)
    {

    }

    explicit AABB(NBPoint start, NBPoint end)
        : minPoint(start)
        , maxPoint(end)
    {

    }*/

    NBPoint minPoint;
    NBPoint maxPoint;

    bool isPointInside(NBPoint pos) noexcept
    {
        return (pos.x >= minPoint.x && pos.y >= minPoint.y) && (pos.x <= maxPoint.x && pos.y <= maxPoint.y);
    }
};
