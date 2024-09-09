
#ifndef SRC_MATH_VECTOR2_HPP
#define SRC_MATH_VECTOR2_HPP

#include <cmath>

namespace nb
{
    namespace Math
    {
        class Vector2
        {

        public:
            constexpr Vector2() noexcept = default;

            constexpr Vector2(const float x, const float y) noexcept
                : x(x), y(y)
            {
            }

            constexpr Vector2(const Vector2 &oth) noexcept = default;

            constexpr Vector2 operator+(const Vector2 &oth) noexcept
            {
                return {this->x + oth.x, this->y + oth.y};
            }
            constexpr Vector2 operator-(const Vector2 &oth) noexcept
            {
                return {this->x - oth.x, this->y - oth.y};
            }
            constexpr Vector2 operator*(const float scalar) noexcept
            {
                return {this->x * scalar, this->y * scalar};
            }

            // if division by zero occure return same vector
            constexpr Vector2 operator/(const float scalar) noexcept
            {
                return static_cast<bool>(scalar) ? Vector2(this->x / scalar, this->y / scalar) : *this;
            }

            constexpr float dot(const Vector2 &oth) noexcept
            {
                return {this->x * oth.x + this->y * oth.y};
            }
            constexpr float cross(const Vector2 &oth) noexcept
            {
                return {this->x * oth.y - this->y * oth.x};
            }

            void normalize() noexcept;
            float length() noexcept;

            float x = 0.0f;
            float y = 0.0f;
        };

    };
};

#endif