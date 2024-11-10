#ifndef SRC_MATH_MATRIX_TRANSFORMATION_HPP
#define SRC_MATH_MATRIX_TRANSFORMATION_HPP

#include "Matrix.hpp"

#include "../Vector2.hpp"
#include "../Vector3.hpp"

namespace nb
{
    namespace Math
    {
        template<typename T>
        constexpr Matrix<T,4, 4> scale(Matrix<T, 4, 4> mat, const Vector3<T>& vec)
        {
            mat *=  Matrix<T,4, 4>({
                        {vec.x, 0, 0, 0},
                        {0, vec.y, 0, 0},
                        {0, 0, vec.z, 0},
                        {0, 0, 0, 1}
                    });
            return mat;
        }

        template<typename T>
        constexpr Matrix<T,3, 3> scale(Matrix<T, 3, 3> mat, const Vector2<T>& vec)
        {
            mat *=  Matrix<T,3, 3>({
                       {vec.x, 0, 0},
                       {0, vec.y, 0},
                       {0, 0, 1}
                    });
            return mat;
        }

        template<typename T>
        constexpr Matrix<T,4, 4> translate(Matrix<T, 4, 4> mat, const Vector3<T>& vec)
        {
            mat *=  Matrix<T,4, 4>({
                        {1, 0, 0, vec.x},
                        {0, 1, 0, vec.y},
                        {0, 0, 1, vec.z},
                        {0, 0, 0, 1}
                    });
            return mat;
        }

        template<typename T>
        constexpr Matrix<T,3, 3> translate(Matrix<T, 3, 3> mat, const Vector2<T>& vec)
        {
            mat *=  Matrix<T,3, 3>({
                        {1, 0, vec.x},
                        {0, 1, vec.y},
                        {0, 0, 1}
                    });
            return mat;
        }

        template<typename T>
        constexpr Matrix<T, 4, 4> rotate(Matrix<T, 4, 4> mat, const Vector3<T>& vec, const float angle)
        {
            const T cos = std::cos(angle);
            const T sin = std::sin(angle);
          
            mat *=  Matrix<T,4,4>({
                        {// first row
                            cos + (1 - cos) * vec.x * vec.x, 
                            (1 - cos) * vec.x * vec.y - sin * vec.z,
                            (1 - cos) * vec.x * vec.z + sin * vec.y,
                            0
                        },
                        {// second row
                            (1 - cos) * vec.y * vec.x + sin * vec.z,
                            cos + (1 - cos) * vec.y * vec.y,
                            (1 - cos) * vec.y * vec.z - sin * vec.x,
                            0
                        },
                        {// third row
                            (1 - cos) * vec.z * vec.x - sin * vec.y,
                            (1 - cos) * vec.z * vec.y + sin * vec.x,
                            cos + (1 - cos) * vec.z * vec.z,
                            0
                        },
                        {// fourh row
                            0,
                            0,
                            0,
                            1
                        }

                    });
            return mat;
        }

        template<typename T>
        constexpr Matrix<T, 4, 4> projection(const T& fov, const T& aspectRatio, const T& nearPlane, const T& farPlane)
        {
            return Matrix<T, 4, 4>({
                { (1.0f / (std::tan(fov / 2.0f) * aspectRatio)), 0, 0, 0},
                {0, (1.0f / std::tan(fov / 2.0f)), 0, 0},
                {0, 0, -1.0f * ((farPlane + nearPlane) / (farPlane - nearPlane)), ((-2 * farPlane * nearPlane) / (farPlane - nearPlane))},
                {0, 0, -1.0f, 0}
            });
        }

        template<typename T>
        constexpr Matrix<T, 4, 4> lookAt(Vector3<T> eye, Vector3<T> center, Vector3<T> up) {
            Vector3<T> z = (center - eye);
            z.normalize();
            Vector3<T> x = up.cross(z);   
            z.normalize();
            Vector3<T> y = z.cross(x); 

            return Matrix<T, 4, 4>({
                {x.x, y.x, z.x, 0.0f},
                {x.y, y.y, z.y, 0.0f},
                {x.z, y.z, z.z, 0.0f},
                {-x.dot(eye), -y.dot(eye), -z.dot(eye), 1.0f}
        });
}

    };
};

#endif

