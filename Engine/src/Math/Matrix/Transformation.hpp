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
            mat *=  {
                        {vec.x, 0, 0, 0},
                        {0, vec.y, 0, 0},
                        {0, 0, vec.z, 0},
                        {0, 0, 0, 1}
                    };
            return mat;
        }

        template<typename T>
        constexpr Matrix<T,3, 3> scale(Matrix<T, 3, 3> mat, const Vector2<T>& vec)
        {
            mat *=  {
                       {vec.x, 0, 0},
                       {0, vec.y, 0},
                       {0, 0, 1}
                    };
            return mat;
        }

        template<typename T>
        constexpr Matrix<T,4, 4> translate(Matrix<T, 4, 4> mat, const Vector3<T>& vec)
        {
            mat *=  {
                        {1, 0, 0, vec.x},
                        {0, 1, 0, vec.y},
                        {0, 0, 1, vec.z},
                        {0, 0, 0, 1}
                    };
            return mat;
        }

        template<typename T>
        constexpr Matrix<T,3, 3> translate(Matrix<T, 3, 3>, const Vector2<T>& vec)
        {
            mat *=  {
                        {1, 0, vec.x},
                        {0, 1, vec.y},
                        {0, 0, 1}
                    };
            return mat;
        }

        template<typename T>
        constexpr Matrix<T, 4, 4> rotate(Matrix<T, 4, 4>, const Vector3<T>& vec, const float angle)
        {
            constexpr T cos = std::cos(angle);
            constexpr T sin = std::sin(angle);
          
            mat *=  {
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

                    };
            return mat;
        }

    };
};

#endif

