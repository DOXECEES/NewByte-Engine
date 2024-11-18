#ifndef SRC_MATH_MATH_HPP
#define SRC_MATH_MATH_HPP

#include "Constants.hpp"
#include "Vector3.hpp"

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

        inline Vector3<float> toFloatColor(const Vector3<uint8_t>& color)
        {
            return {
                static_cast<float>(color.x) / 255.0f,
                static_cast<float>(color.y) / 255.0f,
                static_cast<float>(color.z) / 255.0f
            };
        }

        inline Vector3<uint8_t> toLinearColor(const Vector3<float>& color)
        {
            return {
                static_cast<uint8_t>(color.x * 255.0f),
                static_cast<uint8_t>(color.y * 255.0f),
                static_cast<uint8_t>(color.z * 255.0f)
            };
        }

    };
};

#endif