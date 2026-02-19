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

        class Line
        {

        };

        class Ray
        {
        public:
            Ray() noexcept = default;
            Ray(const Math::Vector3<float>& origin, const Math::Vector3<float>& direction)
                : origin(origin)
                , direction(direction)
            {}

            Math::Vector3<float> pointByParameter(const float t) const noexcept
            {
                return origin + (direction * t);
            }

            const Math::Vector3<float>& getOrigin() const noexcept
            {
                return origin;
            }

            const Math::Vector3<float>& getDirection() const noexcept
            {
                return direction;
            }

        private:
            Math::Vector3<float> origin     = {};
            Math::Vector3<float> direction  = {};
        };

      
        inline float toRadians(const float degrees) noexcept
        {
            return static_cast<float>(degrees * (Constants::PI / Constants::HALF_OF_CIRCLE_IN_DEG));
        } 

        inline float toDegrees(const float radians) noexcept
        {
            return static_cast<float>(radians * (Constants::HALF_OF_CIRCLE_IN_DEG / Constants::PI));
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

        
        template<typename Vec>
        typename Vec::value_type projectVectorToVector(const Vec& a, const Vec& b)
        {
            using value_type = typename Vec::value_type;
            
            const value_type scalar = a.dot(b);
            const value_type length = b.length(); 
            
            if (length <= std::numeric_limits<value_type>::epsilon())  
                return value_type(0);

            return scalar / length;
        }

        inline float squaredDistanceFromPointToRay(const Ray& ray, const Math::Vector3<float>& point) noexcept
        {
            Math::Vector3<float> diff = point - ray.getOrigin();
            float proj = projectVectorToVector(diff, ray.getDirection());
            return (diff.squaredLength()) - (proj * proj);
        }

        inline float distanceFromPointToRay(const Ray& ray, const Math::Vector3<float>& point) noexcept
        {
            return std::sqrtf(squaredDistanceFromPointToRay(ray, point));
        }

        inline float squaredDistanceBetweenRays(const Ray& ray1, const Ray& ray2) noexcept
        {
            const Vector3<float>& dir1 = ray1.getDirection();
            const Vector3<float>& dir2 = ray2.getDirection();
            const Vector3<float>& origin1 = ray1.getOrigin();
            const Vector3<float>& origin2 = ray2.getOrigin();
        
            const float dot12 = dir1.dot(dir2);
            const float lenSq1 = dir1.squaredLength();
            const float lenSq2 = dir2.squaredLength();
        
            float denom = dot12 * dot12 - lenSq1 * lenSq2;
        
            const Vector3<float> originDiff = origin2 - origin1;
        
            if (std::abs(denom) <= std::numeric_limits<float>::epsilon()) // если паралельны
            {
                return originDiff.cross(dir1).squaredLength() / lenSq1;
            }
        
            denom = 1.0f / denom;
        
            const float a = -lenSq2;
            const float b = dot12;
            const float c = -dir1.dot(dir2);
            const float d = lenSq1;
        
            const float e = originDiff.dot(dir1);
            const float f = originDiff.dot(dir2);
        
            const float t1 = (a * e + b * f) * denom;
            const float t2 = (c * e + d * f) * denom;
        
            const Vector3<float> point1 = ray1.pointByParameter(t1);
            const Vector3<float> point2 = ray2.pointByParameter(t2);
        
            return (point1 - point2).squaredLength();
        }
        
        inline float distanceBetweenRays(const Ray& ray1, const Ray& ray2) noexcept
        {
            return std::sqrtf(squaredDistanceBetweenRays(ray1, ray2));
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

            return { 
                Plane(toVector3(left), left.w)
                , Plane(toVector3(right), right.w)
                , Plane(toVector3(bottom), bottom.w)
                , Plane(toVector3(top), top.w)
                , Plane(toVector3(_near), _near.w)
                , Plane(toVector3(_far), _far.w)     
            };
        }

       

        //template<typename Vec>
        //Vec normalize(const Vec& vector) noexcept
        //{
        //    return vector.normalize
        //}
    };
};

#endif