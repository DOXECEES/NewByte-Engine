
#ifndef SRC_MATH_VECTOR2_HPP
#define SRC_MATH_VECTOR2_HPP

#include <cmath>

#include "Constants.hpp"

#include "../Core.hpp"


namespace nb
{
    namespace Math
    {
        template <typename T, typename Enable = void>
        class Vector2;

        template<typename T>
        class Vector2<T, typename std::enable_if<std::is_arithmetic<T>::value>::type>
        {

        public:
            using value_type = T;

            constexpr Vector2() noexcept = default;
            
            constexpr Vector2(const T value) noexcept
                : x(value), y(value) {}

            constexpr Vector2(const T x, const T y) noexcept
                : x(x), y(y) {}

            constexpr Vector2(std::initializer_list<T> list)
            {
                auto it = list.begin();
                x = (it != list.end()) ? *it++ : 0;
                y = (it != list.end()) ? *it++ : 0;
            }

            T& operator[](const size_t index)
            {
                assert(!(index >= 2) && "Index out of range");
                return (index == 0) ? x : y;
            }

            const T& operator[](const size_t index) const
            {
                assert(!(index >= 2) && "Index out of range");
                return (index == 0) ? x : y;
            }

            friend constexpr Vector2<T> operator+(Vector2<T> a, const Vector2<T>& oth) noexcept
            {
                a.x += oth.x;
                a.y += oth.y;
                return a;
            }

            constexpr Vector2<T>& operator+=(const Vector2<T>& oth) noexcept
            {
                this->x += oth.x;
                this->y += oth.y;
                return *this;
            }
            //
            
            //
            friend constexpr Vector2<T> operator-(Vector2<T> a, const Vector2<T>& oth) noexcept
            {
                a.x -= oth.x;
                a.y -= oth.y;
                return a;
            }

            constexpr Vector2<T>& operator-=(const Vector2<T>& oth) noexcept
            {
                this->x -= oth.x;
                this->y -= oth.y;
                return *this;
            }
            //
            
            //
            friend constexpr Vector2<T> operator*(Vector2<T> a, const T scalar) noexcept
            {
                a.x *= scalar;
                a.y *= scalar;
                return a;
            }

            constexpr Vector2<T>& operator*=(const T scalar) noexcept
            {
                this->x *= scalar;
                this->y *= scalar;
                return *this;
            }
            //
            
            //
            friend constexpr Vector2<T> operator/(Vector2<T> a, const T scalar) noexcept
            {
                assert(scalar == T(0));
                a.x /= scalar;
                a.y /= scalar;
                return a;
            }

            constexpr Vector2<T>& operator/=(const T scalar) noexcept
            {
                assert(scalar != T(0));
                this->x /= scalar;
                this->y /= scalar;
                return *this;
            }
            //

            constexpr bool operator==(const Vector2<T>& other) const
            {
                return x == other.x && y == other.y;
            }

            constexpr bool operator!=(const Vector2<T>& other) const
            {
                return x != other.x || y != other.y;
            }

            constexpr T dot(const Vector2<T>& oth) const noexcept 
            {
                return { this->x * oth.x + this->y * oth.y };
            }

            constexpr T cross(const Vector2<T>& oth) const noexcept
            {
                return { this->x * oth.y - this->y * oth.x };
            }

            constexpr bool isOrtogonal(const Vector2<T>& oth) const noexcept
            {
                return (this->dot(oth) == 0);
            }

            constexpr bool isColleniar(const Vector2<T>& oth) const noexcept
            {
                return (this->cross(oth) == 0);
            }

            constexpr float distance(const Vector2<T>& oth) const noexcept
            {
                return std::sqrtf((this->x - oth.x) * (this->x - oth.x) +  (this->y - oth.y) * (this->y - oth.y));
            }

            float getAngleInRadians(const Vector2<T>& b) const noexcept
            {
                return std::acos((this->dot(b)) / (this->length() * b.length()));
            }

            float getAngleInDegrees(const Vector2<T>& b)
            {
                return getAngleInRadians(b) * (180 / Constants::PI);
            }



            void normalize() noexcept
            {
                const float length = this->length();
                if (std::fabs(length) > std::numeric_limits<float>::epsilon())
                {
                    *this /= length;
                }
            }

            float length() const noexcept
            {
                return std::sqrt(dot(*this));
            }

            float squaredLength() const noexcept
            {
                return dot(*this);
            }

            T x = { };
            T y = { };
           
        };

    };
};
#endif
