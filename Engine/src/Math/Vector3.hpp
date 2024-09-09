
#ifndef SRC_MATH_VECTOR3_HPP
#define SRC_MATH_VECTOR3_HPP

#include <cmath>

namespace nb
{
    namespace Math
    {
        class Vector3
        {

        public:
            constexpr Vector3() noexcept = default;

            constexpr Vector3(const float x, const float y, const float z) noexcept
                : x(x), y(y), z(z)
            {
            }

            constexpr Vector3(const Vector3 &oth) noexcept = default;

            constexpr Vector3 operator+(const Vector3 &oth) noexcept
            {
                return {this->x + oth.x, this->y + oth.y, this->z + oth.z};
            }
            constexpr Vector3 operator-(const Vector3 &oth) noexcept
            {
                return {this->x - oth.x, this->y - oth.y, this->z - oth.z};
            }
            constexpr Vector3 operator*(const float scalar) noexcept
            {
                return {this->x * scalar, this->y * scalar, this->z * scalar};
            }

            // if division by zero occure return same vector
            constexpr Vector3 operator/(const float scalar) noexcept
            {
                return static_cast<bool>(scalar) ? Vector3(this->x / scalar, this->y / scalar, this->z / scalar) : *this;
            }

            constexpr float dot(const Vector3 &oth) noexcept
            {
                return {this->x * oth.x + this->y * oth.y + this->z * oth.z};
            }
            constexpr Vector3 cross(const Vector3 &oth) noexcept
            {
                float dx = this->y * oth.z - this->z * oth.y;
                float dy = this->z * oth.x - this->x * oth.z;
                float dz = this->x * oth.y - this->y * oth.x;

                return {dx, dy, dz};
            }

            void normalize() noexcept;
            float length() noexcept;

            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
        };

    };
};

#endif