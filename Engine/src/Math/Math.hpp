#ifndef SRC_MATH_MATH_HPP
#define SRC_MATH_MATH_HPP

// great winapi
///#undef far
///#undef near

#include <array>

#include "Constants.hpp"

#include "Vector2.hpp" 
#include "Vector3.hpp"
#include "Vector4.hpp"

#include "Plane.hpp"

#include "Matrix/Matrix.hpp"

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
        // vec4
        template <typename T>
        Vector3<T> toVector3(const Vector4<T>& vec)
        {
            return Vector3<T>(vec.x, vec.y, vec.z);
        }

        template <typename T>
        Vector2<T> toVector2(const Vector4<T>& vec)
        {
            return Vector2<T>(vec.x, vec.y);
        }

        // vec3
        template <typename T>
        Vector4<T> toVector4Dir(const Vector3<T>& vec)
        {
            return Vector4<T>(vec.x, vec.y, vec.z, static_cast<T>(1));
        } 

        template <typename T>
        Vector4<T> toVector4Pos(const Vector3<T>& vec)
        {
            return Vector4<T>(vec.x, vec.y, vec.z, static_cast<T>(0));
        } 

        template <typename T>
        Vector2<T> toVector2(const Vector3<T>& vec)
        {
            return Vector2<T>(vec.x, vec.y);
        }

        // vec2
        template <typename T>
        Vector3<T> toVector3(const Vector2<T>& vec)
        {
            return Vector3<T>(vec.x, vec.y, static_cast<T>(0));
        }

        template <typename T>
        Vector4<T> toVector4Dir(const Vector2<T>& vec)
        {
            return Vector4<T>(vec.x, vec.y, static_cast<T>(0), static_cast<T>(1));
        } 

        template <typename T>
        Vector4<T> toVector4Pos(const Vector2<T>& vec)
        {
            return Vector4<T>(vec.x, vec.y, static_cast<T>(0), static_cast<T>(0));
        } 

        inline std::array<Plane, 6> getFrustrumPlanes(const Mat4<float>& projection)
        {
            Vector4<float> p4 = {projection[3]};
            Vector4<float> p3 = {projection[2]};
            Vector4<float> p2 = {projection[1]};
            Vector4<float> p1 = {projection[0]};

            Vector4<float> left = p4 + p1;
            Vector4<float> right = p4 - p1;
            Vector4<float> bottom = p4 + p2;
            Vector4<float> top = p4 - p2;
            Vector4<float> _near = p4 + p3;
            Vector4<float> _far = p4 - p3;

            left.normalize();
            right.normalize();
            bottom.normalize();
            top.normalize();
            _near.normalize();
            _far.normalize();

            return { Plane(toVector3(left), left.w)
                   , Plane(toVector3(right), right.w)
                   , Plane(toVector3(bottom), bottom.w)
                   , Plane(toVector3(top), top.w)
                   , Plane(toVector3(_near), _near.w)
                   , Plane(toVector3(_far), _far.w)     
            };
        }


    };
};

#endif