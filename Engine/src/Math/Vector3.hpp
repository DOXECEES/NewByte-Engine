
#ifndef SRC_MATH_VECTOR3_HPP
#define SRC_MATH_VECTOR3_HPP

#include <cmath>
#include <Reflection/Reflection.hpp>
#include "Constants.hpp"
#include <Math/NbMath.hpp>

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
            using value_type = T;

            constexpr Vector3() noexcept = default;

            constexpr Vector3(const T value) noexcept
                :x(value), y(value), z(value) {}

            constexpr Vector3(const T x, const T y, const T z) noexcept
                : x(x), y(y), z(z) {}

            constexpr Vector3(std::initializer_list<T> list)
            {
                auto it = list.begin();
                x = (it != list.end()) ? *it++ : 0;
                y = (it != list.end()) ? *it++ : 0;
                z = (it != list.end()) ? *it++ : 0;
            }

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
            constexpr Vector3<T> operator-() noexcept
            {
                return Vector3<T>(-this->x, -this->y, -this->z);
            }
        


            //
            friend constexpr Vector3<T> operator*(Vector3<T> a, const T scalar) noexcept
            {
                a.x *= scalar;
                a.y *= scalar;
                a.z *= scalar;
                return a;
            }

            friend constexpr Vector3<T> operator*(Vector3<T> a, const Vector3<T>& b) noexcept
            {
                a.x *= b.x;
                a.y *= b.y;
                a.z *= b.z;
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
                if constexpr (std::is_floating_point_v<T>)
                {
                    assert(!nbstl::Math::equalByEpsilon(scalar, T(0)) && "Vector3: division by zero (scalar is too close to epsilon)");

                    T invScalar = static_cast<T>(1.0) / scalar;
                    a.x *= invScalar;
                    a.y *= invScalar;
                    a.z *= invScalar;
                }
                else
                {
                    assert(scalar != 0 && "Vector3: division by zero (integer)");

                    a.x /= scalar;
                    a.y /= scalar;
                    a.z /= scalar;
                }
                return a;
            }

            constexpr Vector3<T>& operator/=(const T scalar) noexcept
            {
                if constexpr (std::is_floating_point_v<T>)
                {
                    assert(!nbstl::Math::equalByEpsilon(scalar, T(0)) && "Vector3: division by zero (scalar is too close to epsilon)");

                    T invScalar = static_cast<T>(1.0) / scalar;
                    this->x *= invScalar;
                    this->y *= invScalar;
                    this->z *= invScalar;
                }
                else
                {
                    assert(scalar != 0 && "Vector3: division by zero (integer)");

                    this->x /= scalar;
                    this->y /= scalar;
                    this->z /= scalar;
                }

                return *this;
            }
            //
            constexpr bool operator==(const Vector3<T>& other) const
            {
                if constexpr (std::is_floating_point_v<T>)
                {
                    return nbstl::Math::equalByEpsilon(x, other.x) 
                        && nbstl::Math::equalByEpsilon(y, other.y) 
                        && nbstl::Math::equalByEpsilon(z, other.z);
                }
                else
                {
                    return x == other.x 
                        && y == other.y
                        && z == other.z;
                }
            }

            constexpr bool operator!=(const Vector3<T>& other) const
            {
                if constexpr (std::is_floating_point_v<T>)
                {
                    return !nbstl::Math::equalByEpsilon(x, other.x)
                        || !nbstl::Math::equalByEpsilon(y, other.y)
                        || !nbstl::Math::equalByEpsilon(z, other.z);
                }
                else
                {
                    return x != other.x
                        || y != other.y
                        || z != other.z;
                }
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
                if constexpr (std::is_floating_point_v<T>)
                {
                    return nbstl::Math::equalByEpsilon(this->dot(oth), 0);
                }
                else
                {
                    return (this->dot(oth) == 0);
                }
            }

            constexpr bool isColleniar(const Vector3<T>& oth) const noexcept
            {
                if constexpr (std::is_floating_point_v<T>)
                {
                    return nbstl::Math::equalByEpsilon(this->cross(oth), 0);
                }
                else
                {
                    return (this->cross(oth) == 0);
                }
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

            float squaredLength() const noexcept
            {
                return dot(*this);
            }

            T x = { };
            T y = { };
            T z = { };
        };
    };
};

namespace nb::Reflect
{

    template <typename T> struct Reflect<nb::Math::Vector3<T>>
    {
        inline static const char* name = []()
        {
            if constexpr (std::is_same_v<T, float>)
            {
                return "Vector3<float>";
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                return "Vector3<double>";
            }
            else if constexpr (std::is_same_v<T, int>)
            {
                return "Vector3<int>";
            }
            else
            {
                return "Vector3<unknown>";
            }
        }();

        static constexpr auto fields = std::make_tuple(
            NB_FIELD(nb::Math::Vector3<T>, x),
            NB_FIELD(nb::Math::Vector3<T>, y),
            NB_FIELD(nb::Math::Vector3<T>, z)
        );

        static constexpr bool isInternal = false;
    };

};

#endif
