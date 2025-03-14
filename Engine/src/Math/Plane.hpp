#ifndef SRC_MATH_PLANE_HPP
#define SRC_MATH_PLANE_HPP

#include "Vector3.hpp"

namespace nb 
{
    namespace Math
    {
        class Plane
        {
            public:
                explicit Plane(const Vector3<float>& point1,
                        const Vector3<float>& point2,
                        const Vector3<float>& point3)
                {
                    Vector3<float> a = point2 - point1;
                    Vector3<float> b = point3 - point1;
                    normal = a.cross(b);
                    normal.normalize();
                    source = point1;
                }
                
                explicit Plane(const Vector3<float>& point, const Vector3<float> normal)
                    : normal(normal)
                    , source(normal)
                {

                }

                explicit Plane(const Vector3<float>& normal, const float distance)
                    : normal(normal)
                    , source(normal * distance)
                {

                }

            //private:
                Vector3<float> normal;
                Vector3<float> source;
        };
    };
};

#endif