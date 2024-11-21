
#ifndef SRC_MATH_VECTOR3_HPP
#define SRC_MATH_VECTOR3_HPP

#include <cmath>

#include "Constants.hpp"
#include "../Core.hpp"

namespace nb
{
    namespace Math
    {
        template <typename T, typename Enable = void>
        class Vector3;

        template<typename T>
        class Vector3<T, typename std::enable_if<std::is_arithmetic<T>::value>::type>
        {

        public:
            constexpr Vector3() noexcept = default;

            constexpr Vector3(const T value) noexcept
                :x(value), y(value), z(value) {}

            constexpr Vector3(const T x, const T y, const T z) noexcept
                : x(x), y(y), z(z) {}


            T& operator[](const size_t index)
            {
                assert(!(index >= 3) && "Index out of range");
                return (index == 0) ? x : (index == 1) ? y : z;
            }

            const T& operator[](const size_t index) const
            {
                assert(!(index >= 3) && "Index out of range");
                return (index == 0) ? x : (index == 1) ? y : z;
            }

            friend constexpr Vector3<T> operator+(Vector3<T> a, const Vector3<T>& oth) noexcept
            {
                a.x += oth.x;
                a.y += oth.y;
                a.z += oth.z;
                return a;
            }

            constexpr Vector3<T>& operator+=(const Vector3<T>& oth) noexcept
            {
                this->x += oth.x;
                this->y += oth.y;
                this->z += oth.z;
                return *this;
            }
            //

            //
            friend constexpr Vector3<T> operator-(Vector3<T> a, const Vector3<T>& oth) noexcept
            {
                a.x -= oth.x;
                a.y -= oth.y;
                a.z -= oth.z;
                return a;
            }

            constexpr Vector3<T>& operator-=(const Vector3<T>& oth) noexcept
            {
                this->x -= oth.x;
                this->y -= oth.y;
                this->z -= oth.z;
                return *this;
            }
            //

            //
            friend constexpr Vector3<T> operator*(Vector3<T> a, const T scalar) noexcept
            {
                a.x *= scalar;
                a.y *= scalar;
                a.z *= scalar;
                return a;
            }

            constexpr Vector3<T>& operator*=(const T scalar) noexcept
            {
                this->x *= scalar;
                this->y *= scalar;
                this->z *= scalar;
                return *this;
            }
            //

            //
            friend constexpr Vector3<T> operator/(Vector3<T> a, const T scalar) noexcept
            {
                assert(scalar == T(0));
                a.x /= scalar;
                a.y /= scalar;
                a.z /= scalar;
                return a;
            }

            constexpr Vector3<T>& operator/=(const T scalar) noexcept
            {
                assert(scalar != T(0));
                this->x /= scalar;
                this->y /= scalar;
                this->z /= scalar;
                return *this;
            }
            //
            constexpr bool operator==(const Vector3<T>& other) const
            {
                return x == other.x && y == other.y && z == other.z;
            }

            constexpr bool operator!=(const Vector3<T>& other) const
            {
                return x != other.x || y != other.y || z != other.z;
            }

            constexpr T dot(const Vector3<T>& oth) const noexcept
            {
                return { this->x * oth.x + this->y * oth.y + this->z * oth.z };
            }

            constexpr Vector3<T> cross(const Vector3<T>& oth) const noexcept
            {
                return { this->y * oth.z - this->z * oth.y
                        , this->z * oth.x - this->x * oth.z
                        , this->x * oth.y - this->y * oth.x};
            }

            constexpr bool isOrtogonal(const Vector3<T>& oth) const noexcept
            {
                return (this->dot(oth) == 0);
            }

            constexpr bool isColleniar(const Vector3<T>& oth) const noexcept
            {
                return (this->cross(oth) == 0);
            }

            float getAngleInRadians(const Vector3<T>& b) const noexcept
            {
                return std::acos((this->dot(b)) / (this->length() * b.length()));
            }

            float getAngleInDegrees(const Vector3<T>& b)
            {
                return getAngleInRadians(b) * (180 / Constants::PI);
            }

            void normalize() noexcept
            {
                const float length = this->length();
                if (length != 0)
                {
                    *this /= length;
                }
            }

            float length() const noexcept
            {
                return std::sqrt(dot(*this));
            }

            T x = { };
            T y = { };
            T z = { };
        };

    };
};

#endif
