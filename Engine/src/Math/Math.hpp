#ifndef SRC_MATH_MATH_HPP
#define SRC_MATH_MATH_HPP

#include "Constants.hpp"

namespace nb
{
    namespace Math
    {
        inline float toRadians(const float degrees) noexcept
        {
            return degrees * (Constants::PI / Constants::HALF_OF_CIRCLE_IN_DEG);
        } 

        inline float toDegrees(const float radians) noexcept
        {
            return radians * (Constants::HALF_OF_CIRCLE_IN_DEG / Constants::PI);
        }
    };
};

#endif