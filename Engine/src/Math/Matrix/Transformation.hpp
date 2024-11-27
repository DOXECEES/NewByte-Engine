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
        constexpr T det3(const Matrix<T, 3, 3>& mat)
        {
             return mat[0][0] * (mat[1][1] * mat[2][2] - mat[1][2] * mat[2][1]) +
                    mat[0][1] * (mat[1][2] * mat[2][0] - mat[1][0] * mat[2][2]) +
                    mat[0][2] * (mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0]);
        }

        template<typename T>
        constexpr T det(const Matrix<T, 4, 4>& mat)
        {
            return mat[0][0] * det3<T>({
                {mat[1][1], mat[1][2], mat[1][3]},
                {mat[2][1], mat[2][2], mat[2][3]},
                {mat[3][1], mat[3][2], mat[3][3]}
            }) -
            mat[0][1] * det3<T>({
                {mat[1][0], mat[1][2], mat[1][3]},
                {mat[2][0], mat[2][2], mat[2][3]},
                {mat[3][0], mat[3][2], mat[3][3]}
            }) +
            mat[0][2] * det3<T>({
                {mat[1][0], mat[1][1], mat[1][3]},
                {mat[2][0], mat[2][1], mat[2][3]},
                {mat[3][0], mat[3][1], mat[3][3]}
            }) -
            mat[0][3] * det3<T>({
                {mat[1][0], mat[1][1], mat[1][2]},
                {mat[2][0], mat[2][1], mat[2][2]},
                {mat[3][0], mat[3][1], mat[3][2]}
            });
        }


        template<typename T>
        Matrix<T, 4, 4> inverse(const Matrix<T, 4, 4>& mat) {
            T d = det(mat);
            if (d == 0) {
                //throw std::runtime_error("Matrix is singular and cannot be inverted.");
            }

            Matrix<T, 4, 4> inv;
            T invDet = 1 / d;

            // Заполнение матрицы обратных элементов (матрица алгебраических дополнений)
            inv[0][0] = invDet * det3<T>({
                {mat[1][1], mat[1][2], mat[1][3]},
                {mat[2][1], mat[2][2], mat[2][3]},
                {mat[3][1], mat[3][2], mat[3][3]}
            });
            inv[0][1] = -invDet * det3<T>({
                {mat[1][0], mat[1][2], mat[1][3]},
                {mat[2][0], mat[2][2], mat[2][3]},
                {mat[3][0], mat[3][2], mat[3][3]}
            });
            inv[0][2] = invDet * det3<T>({
                {mat[1][0], mat[1][1], mat[1][3]},
                {mat[2][0], mat[2][1], mat[2][3]},
                {mat[3][0], mat[3][1], mat[3][3]}
            });
            inv[0][3] = -invDet * det3<T>({
                {mat[1][0], mat[1][1], mat[1][2]},
                {mat[2][0], mat[2][1], mat[2][2]},
                {mat[3][0], mat[3][1], mat[3][2]}
            });

            inv[1][0] = -invDet * det3<T>({
                {mat[0][1], mat[0][2], mat[0][3]},
                {mat[2][1], mat[2][2], mat[2][3]},
                {mat[3][1], mat[3][2], mat[3][3]}
            });
            inv[1][1] = invDet * det3<T>({
                {mat[0][0], mat[0][2], mat[0][3]},
                {mat[2][0], mat[2][2], mat[2][3]},
                {mat[3][0], mat[3][2], mat[3][3]}
            });
            inv[1][2] = -invDet * det3<T>({
                {mat[0][0], mat[0][1], mat[0][3]},
                {mat[2][0], mat[2][1], mat[2][3]},
                {mat[3][0], mat[3][1], mat[3][3]}
            });
            inv[1][3] = invDet * det3<T>({
                {mat[0][0], mat[0][1], mat[0][2]},
                {mat[2][0], mat[2][1], mat[2][2]},
                {mat[3][0], mat[3][1], mat[3][2]}
            });

            inv[2][0] = invDet * det3<T>({
                {mat[0][1], mat[0][2], mat[0][3]},
                {mat[1][1], mat[1][2], mat[1][3]},
                {mat[3][1], mat[3][2], mat[3][3]}
            });
            inv[2][1] = -invDet * det3<T>({
                {mat[0][0], mat[0][2], mat[0][3]},
                {mat[1][0], mat[1][2], mat[1][3]},
                {mat[3][0], mat[3][2], mat[3][3]}
            });
            inv[2][2] = invDet * det3<T>({
                {mat[0][0], mat[0][1], mat[0][3]},
                {mat[1][0], mat[1][1], mat[1][3]},
                {mat[3][0], mat[3][1], mat[3][3]}
            });
            inv[2][3] = -invDet * det3<T>({
                {mat[0][0], mat[0][1], mat[0][2]},
                {mat[1][0], mat[1][1], mat[1][2]},
                {mat[3][0], mat[3][1], mat[3][2]}
            });

            inv[3][0] = -invDet * det3<T>({
                {mat[0][1], mat[0][2], mat[0][3]},
                {mat[1][1], mat[1][2], mat[1][3]},
                {mat[2][1], mat[2][2], mat[2][3]}
            });
            inv[3][1] = invDet * det3<T>({
                {mat[0][0], mat[0][2], mat[0][3]},
                {mat[1][0], mat[1][2], mat[1][3]},
                {mat[2][0], mat[2][2], mat[2][3]}
            });
            inv[3][2] = -invDet * det3<T>({
                {mat[0][0], mat[0][1], mat[0][3]},
                {mat[1][0], mat[1][1], mat[1][3]},
                {mat[2][0], mat[2][1], mat[2][3]}
            });
            inv[3][3] = invDet * det3<T>({
                {mat[0][0], mat[0][1], mat[0][2]},
                {mat[1][0], mat[1][1], mat[1][2]},
                {mat[2][0], mat[2][1], mat[2][2]}
            });

            return inv;
        }


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
                        {1, 0, 0, 0},
                        {0, 1, 0, 0},
                        {0, 0, 1, 0},
                        {vec.x, vec.y, vec.z, 1}
                    });
            return mat;
        }

        template<typename T>
        constexpr Matrix<T,3, 3> translate(Matrix<T, 3, 3> mat, const Vector2<T>& vec)
        {
            mat *=  Matrix<T,3, 3>({
                        {1, 0, 0},
                        {0, 1, 0},
                        {vec.x, vec.y, 1}
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
        constexpr Vector3<T> rotate(const Quaternion<T>& q, const Vector3<T>& v)
        {
            return q * v;

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
        constexpr Matrix<T, 4, 4> lookAt(Vector3<T> eye, Vector3<T> center, Vector3<T> up)
        {
            Vector3<T> z = (center - eye);
            z.normalize();  

            // Проверяем на коллинеарность
            Vector3<T> x = up.cross(z);
            if (x.length() < std::numeric_limits<T>::epsilon())
            {
                up = (up != Vector3<T>(0, 0, 1)) ? Vector3<T>(0, 0, 1) : Vector3<T>(0, 1, 0);
                x = up.cross(z);
            }
            x.normalize(); 

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

