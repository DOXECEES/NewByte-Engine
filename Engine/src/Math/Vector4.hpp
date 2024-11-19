#ifndef SRC_MATH_VECTOR4_HPP
#define SRC_MATH_VECTOR4_HPP

#include <cmath>

#include "Constants.hpp"
#include "../Core.hpp"

namespace nb
{
    namespace Math
    {
        template <typename T, typename Enable = void>
        class Vector4;

        template<typename T>
        class Vector4<T, typename std::enable_if<std::is_arithmetic<T>::value>::type>
        {

        public:
            constexpr Vector4() noexcept = default;

            constexpr Vector4(const T value) noexcept
                :x(value), y(value), z(value), w(value) {}

            constexpr Vector4(const T x, const T y, const T z, const T w) noexcept
                : x(x), y(y), z(z), w(w) {}


            T& operator[](const size_t index) 
            {
                assert(!(index >= 4) && "Index out of range");
                return (index == 0) ? x : (index == 1) ? y : (index == 2) ? z : w;
            }

            const T& operator[](const size_t index) const 
            {
                assert(!(index >= 4) && "Index out of range");
                return (index == 0) ? x : (index == 1) ? y : (index == 2) ? z : w;
            }

            friend constexpr Vector4<T> operator+(Vector4<T> a, const Vector4<T>& oth) noexcept
            {
                a.x += oth.x;
                a.y += oth.y;
                a.z += oth.z;
                a.w += oth.w;
                return a;
            }

            constexpr Vector4<T>& operator+=(const Vector4<T>& oth) noexcept
            {
                this->x += oth.x;
                this->y += oth.y;
                this->z += oth.z;
                this->w += oth.w;
                return *this;
            }
            //

            //
            friend constexpr Vector4<T> operator-(Vector4<T> a, const Vector4<T>& oth) noexcept
            {
                a.x -= oth.x;
                a.y -= oth.y;
                a.z -= oth.z;
                a.w -= oth.w;
                return a;
            }

            constexpr Vector4<T>& operator-=(const Vector4<T>& oth) noexcept
            {
                this->x -= oth.x;
                this->y -= oth.y;
                this->z -= oth.z;
                this->w -= oth.w;
                return *this;
            }
            //

            //
            friend constexpr Vector4<T> operator*(Vector4<T> a, const T scalar) noexcept
            {
                a.x *= scalar;
                a.y *= scalar;
                a.z *= scalar;
                a.w *= scalar;
                return a;
            }

            constexpr Vector4<T>& operator*=(const T scalar) noexcept
            {
                this->x *= scalar;
                this->y *= scalar;
                this->z *= scalar;
                this->w *= scalar;
                return *this;
            }
            //

            //
            friend constexpr Vector4<T> operator/(Vector4<T> a, const T scalar) noexcept
            {
                assert(scalar == T(0));
                a.x /= scalar;
                a.y /= scalar;
                a.z /= scalar;
                a.w /= scalar;
                return a;
            }

            constexpr Vector4<T>& operator/=(const T scalar) noexcept
            {
                assert(scalar != T(0));
                this->x /= scalar;
                this->y /= scalar;
                this->z /= scalar;
                this->w /= scalar;
                return *this;
            }
            //

            constexpr bool operator!=(const Vector4<T>& other) const
            {
                return x != other.x || y != other.y || z != other.z || w != other.w;
            }

            constexpr T dot(const Vector4<T>& oth) const noexcept
            {
                return { this->x * oth.x + this->y * oth.y + this->z * z + this->w * oth.w};
            }

            /*constexpr Vector4<T> cross(const Vector4<T>& oth) const noexcept
            {
                return { this->y * oth.z - this->z * oth.y
                        , this->z * oth.x - this->x * oth.z
                        , this->x * oth.y - this->y * oth.x };
            }*/

            constexpr bool isOrtogonal(const Vector4<T>& oth) const noexcept
            {
                return (this->dot(oth) == 0);
            }

           /* constexpr bool isColleniar(const Vector3<T>& oth) const noexcept
            {
                return (this->cross(oth) == 0);
            }*/

            /*float getAngleInRadians(const Vector4<T>& b) const noexcept
            {
                return std::acos((this->dot(b)) / (this->length() * b.length()));
            }

            float getAngleInDegrees(const Vector3<T>& b)
            {
                return getAngleInRadians(b) * (180 / 3.141592f);
            }*/

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
            T w = { };
        };
    };
};

#endif