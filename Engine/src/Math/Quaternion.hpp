#ifndef SRC_MATH_QUATERNION_HPP
#define SRC_MATH_QUATERNION_HPP


#include "Vector3.hpp"
#include "Vector4.hpp"

#include "Matrix/Matrix.hpp"

namespace nb
{
    namespace Math
    {
        template<typename T>
        class Quaternion
        {
            public:
                constexpr Quaternion() noexcept
                    :x(T(0))
                    ,y(T(0))
                    ,z(T(0))
                    ,w(T(1))
                {};
                constexpr Quaternion(const Math::Vector4<T> &vec) noexcept
                    :x(vec.x)
                    ,y(vec.y)
                    ,z(vec.z)
                    ,w(vec.w)
                {}
                constexpr Quaternion(const T& x, const T& y, const T& z, const T& w) noexcept
                    :x(x)
                    ,y(y)
                    ,z(z)
                    ,w(w)
                {}

                constexpr Quaternion(const Quaternion &other) noexcept = default;
                constexpr Quaternion(Quaternion &&other) noexcept = default;
                constexpr Quaternion& operator=(const Quaternion &other) noexcept = default;
                constexpr Quaternion& operator=(Quaternion &&other) noexcept = default;

                friend constexpr Quaternion<T> operator+(Quaternion<T> a, const Quaternion<T>& oth) noexcept
                {
                    a.x += oth.x;
                    a.y += oth.y;
                    a.z += oth.z;
                    a.w += oth.w;
                    return a;
                }

                constexpr Quaternion<T>& operator+=(const Quaternion<T>& oth) noexcept
                {
                    this->x += oth.x;
                    this->y += oth.y;
                    this->z += oth.z;
                    this->w += oth.w;
                    return *this;
                }

                friend constexpr Quaternion<T> operator*(Quaternion<T> a, const Quaternion<T>& oth) noexcept
                {
                    return Quaternion<T>(
                        (a.w * oth.w - a.x * oth.x - a.y * oth.y - a.z * oth.z),
                        (a.w * oth.x + a.x * oth.w + a.y * oth.z - a.z * oth.y),
                        (a.w * oth.y - a.x * oth.z + a.y * oth.w + a.z * oth.x),
                        (a.w * oth.z + a.x * oth.y - a.y * oth.x + a.z * oth.w)
                    );
                }

                template<typename T>
                friend constexpr Math::Vector3<T> operator*(const Quaternion<T>& q, const Math::Vector3<T>& v)
                {
                    Math::Vector3<T> const QuatVector(q.x, q.y, q.z);
                    Math::Vector3<T> const uv = QuatVector.cross(v); 
                    Math::Vector3<T> const uuv = QuatVector.cross(uv); 

                    return v + ((uv * q.w) + uuv) * static_cast<T>(2);
                }

                constexpr Quaternion<T>& operator*=(const Quaternion<T>& oth) noexcept
                {
                    *this = *this * oth;
                    return *this;
                }

                constexpr Quaternion<T> conjugate() const noexcept
                {
                    return Quaternion<T>(-this->x, -this->y, -this->z, this->w);
                }

                constexpr float length() const noexcept
                {
                    return sqrtf(this->x * this->x + this->y * this->y + this->z * this->z + this->w * this->w);
                }

                constexpr void normalize() noexcept
                {
                    const auto magnitude = this->length();
                    if (magnitude > 0.0f)
                    {
                        this->x /= magnitude;
                        this->y /= magnitude;
                        this->z /= magnitude; 
                        this->w /= magnitude;
                    }
                }

                constexpr Quaternion<T> inverse() const noexcept
                {
                    auto magnitude = this->length();

                    if(magnitude > 0.0f)
                    {
                        magnitude *= magnitude;
                        auto con = conjugate();

                        return Quaternion<T>(con.x / magnitude, con.y / magnitude, con.z / magnitude, con.w / magnitude);
                    }
                    return Quaternion<T>();
                }

                constexpr static Quaternion<T> axisAngleToQuaternion(float theta, const Math::Vector3<T>& axis) noexcept
                {
                    T w = std::cos(theta / 2);
                    T s = std::sin(theta / 2);

                    return Quaternion<T>(axis.x * s, axis.y * s, axis.z * s, w);
                }

                constexpr static Quaternion<T> eulerToQuaternion(const float yaw, const float pitch, const float roll) noexcept
                {
                    T cosYaw = std::cos(yaw / 2);
                    T sinYaw = std::sin(yaw / 2);
                    T cosPitch = std::cos(pitch / 2);
                    T sinPitch = std::sin(pitch / 2);
                    T cosRoll = std::cos(roll / 2);
                    T sinRoll = std::sin(roll / 2);

                    T w = static_cast<T>((cosRoll * cosPitch * cosYaw) + (sinRoll * sinPitch * sinYaw));
                    T x = static_cast<T>((sinRoll * cosPitch * cosYaw) - (cosRoll * cosPitch * sinYaw));
                    T y = static_cast<T>((cosRoll * sinPitch * cosYaw) + (sinRoll * cosPitch * sinYaw));
                    T z = static_cast<T>((cosRoll * cosPitch * sinYaw) - (sinRoll * sinPitch * cosYaw));

                    return Quaternion<T>(x, y, z, w);
                }


                Quaternion<T> cross(const Quaternion<T>& oth) const
                {
                    return Quaternion<T>(
                        (this->w * oth.x + this->x * oth.w + this->y * oth.z - this->z * oth.y),
                        (this->w * oth.y + this->y * oth.w + this->z * oth.x - this->x * oth.z),
                        (this->w * oth.z + this->z * oth.w + this->x * oth.y - this->y * oth.x),
                        (this->w * oth.w - this->x * oth.x - this->y * oth.y - this->z * oth.z)
                    );
                }

                constexpr Matrix<T,4,4> toMatrix4() const noexcept {
                    T xx = x * x, yy = y * y, zz = z * z;
                    T xy = x * y, xz = x * z, yz = y * z;
                    T wx = w * x, wy = w * y, wz = w * z;

                    return Matrix<T,4,4>{
                        {1 - 2 * (yy + zz), 2 * (xy - wz), 2 * (xz + wy), 0},
                        {2 * (xy + wz), 1 - 2 * (xx + zz), 2 * (yz - wx), 0},
                        {2 * (xz - wy), 2 * (yz + wx), 1 - 2 * (xx + yy), 0},
                        {0, 0, 0, 1}
                    };
                }

                

            //private:
                T x = T(0);
                T y = T(0);
                T z = T(0);
                T w = T(1);
        };
    };
};

#endif


