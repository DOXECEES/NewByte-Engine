#ifndef SRC_MATH_MATRIX_TRANSFORMATION_HPP
#define SRC_MATH_MATRIX_TRANSFORMATION_HPP

#include <array>

#include "../Vector2.hpp"
#include "../Vector3.hpp"
#include "../Vector4.hpp"

#include "../Quaternion.hpp"


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
        Matrix<T, 4, 4> inverse(const Matrix<T, 4, 4>& m) {
            // // 
            
            // auto A2323 = m[2][2] * m[3][3] - m[2][3] * m[3][2] ;
            // auto A1323 = m[2][1] * m[3][3] - m[2][3] * m[3][1] ;
            // auto A1223 = m[2][1] * m[3][2] - m[2][2] * m[3][1] ;
            // auto A0323 = m[2][0] * m[3][3] - m[2][3] * m[3][0] ;
            // auto A0223 = m[2][0] * m[3][2] - m[2][2] * m[3][0] ;
            // auto A0123 = m[2][0] * m[3][1] - m[2][1] * m[3][0] ;
            // auto A2313 = m[1][2] * m[3][3] - m[1][3] * m[3][2] ;
            // auto A1313 = m[1][1] * m[3][3] - m[1][3] * m[3][1] ;
            // auto A1213 = m[1][1] * m[3][2] - m[1][2] * m[3][1] ;
            // auto A2312 = m[1][2] * m[2][3] - m[1][3] * m[2][2] ;
            // auto A1312 = m[1][1] * m[2][3] - m[1][3] * m[2][1] ;
            // auto A1212 = m[1][1] * m[2][2] - m[1][2] * m[2][1] ;
            // auto A0313 = m[1][0] * m[3][3] - m[1][3] * m[3][0] ;
            // auto A0213 = m[1][0] * m[3][2] - m[1][2] * m[3][0] ;
            // auto A0312 = m[1][0] * m[2][3] - m[1][3] * m[2][0] ;
            // auto A0212 = m[1][0] * m[2][2] - m[1][2] * m[2][0] ;
            // auto A0113 = m[1][0] * m[3][1] - m[1][1] * m[3][0] ;
            // auto A0112 = m[1][0] * m[2][1] - m[1][1] * m[2][0] ;

            // auto det = m[0][0] * ( m[1][1] * A2323 - m[1][2] * A1323 + m[1][3] * A1223 ) 
            //         - m[0][1] * ( m[1][0] * A2323 - m[1][2] * A0323 + m[1][3] * A0223 ) 
            //         + m[0][2] * ( m[1][0] * A1323 - m[1][1] * A0323 + m[1][3] * A0123 ) 
            //         - m[0][3] * ( m[1][0] * A1223 - m[1][1] * A0223 + m[1][2] * A0123 ) ;
            // det = 1.0f / det;

            // return
            // {
            //     {
            //         det *   ( m[1][1] * A2323 - m[1][2] * A1323 + m[1][3] * A1223 ),
            //         det * - ( m[0][1] * A2323 - m[0][2] * A1323 + m[0][3] * A1223 ),
            //         det *   ( m[0][1] * A2313 - m[0][2] * A1313 + m[0][3] * A1213 ),
            //         det * - ( m[0][1] * A2312 - m[0][2] * A1312 + m[0][3] * A1212 )
            //     },
            //     {
            //         det * - ( m[1][0] * A2323 - m[1][2] * A0323 + m[1][3] * A0223 ),
            //         det *   ( m[0][0] * A2323 - m[0][2] * A0323 + m[0][3] * A0223 ),
            //         det * - ( m[0][0] * A2313 - m[0][2] * A0313 + m[0][3] * A0213 ),
            //         det *   ( m[0][0] * A2312 - m[0][2] * A0312 + m[0][3] * A0212 )
            //     },
            //     {
            //         det *   ( m[1][0] * A1323 - m[1][1] * A0323 + m[1][3] * A0123 ),
            //         det * - ( m[0][0] * A1323 - m[0][1] * A0323 + m[0][3] * A0123 ),
            //         det *   ( m[0][0] * A1313 - m[0][1] * A0313 + m[0][3] * A0113 ),
            //         det * - ( m[0][0] * A1312 - m[0][1] * A0312 + m[0][3] * A0112 )
            //     },
            //     {
            //         det * - ( m[1][0] * A1223 - m[1][1] * A0223 + m[1][2] * A0123 ),
            //         det *   ( m[0][0] * A1223 - m[0][1] * A0223 + m[0][2] * A0123 ),
            //         det * - ( m[0][0] * A1213 - m[0][1] * A0213 + m[0][2] * A0113 ),
            //         det *   ( m[0][0] * A1212 - m[0][1] * A0212 + m[0][2] * A0112 )
            //     }
            // };

            T Coef00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
            T Coef02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
            T Coef03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];

            T Coef04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
            T Coef06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
            T Coef07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];

            T Coef08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
            T Coef10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
            T Coef11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];

            T Coef12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
            T Coef14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
            T Coef15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];

            T Coef16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
            T Coef18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
            T Coef19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];

            T Coef20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
            T Coef22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
            T Coef23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

            Vector4<T> Fac0(Coef00, Coef00, Coef02, Coef03);
            Vector4<T> Fac1(Coef04, Coef04, Coef06, Coef07);
            Vector4<T> Fac2(Coef08, Coef08, Coef10, Coef11);
            Vector4<T> Fac3(Coef12, Coef12, Coef14, Coef15);
            Vector4<T> Fac4(Coef16, Coef16, Coef18, Coef19);
            Vector4<T> Fac5(Coef20, Coef20, Coef22, Coef23);
            
            Vector4<T> Vec0(m[1][0], m[0][0], m[0][0], m[0][0]);
            Vector4<T> Vec1(m[1][1], m[0][1], m[0][1], m[0][1]);
            Vector4<T> Vec2(m[1][2], m[0][2], m[0][2], m[0][2]);
            Vector4<T> Vec3(m[1][3], m[0][3], m[0][3], m[0][3]);

            Vector4<T> Inv0(Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
            Vector4<T> Inv1(Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
            Vector4<T> Inv2(Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
            Vector4<T> Inv3(Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);

            Vector4<T> SignA(+1, -1, +1, -1);
            Vector4<T> SignB(-1, +1, -1, +1);
            Mat4<T> Inverse = {
                {Inv0 * SignA},
                {Inv1 * SignB},
                {Inv2 * SignA},
                {Inv3 * SignB}
                };

            Vector4<T> Row0(Inverse[0][0], Inverse[1][0], Inverse[2][0], Inverse[3][0]);
            Vector4<T> Dot0(m[0] * Row0);
            T Dot1 = (Dot0[0] + Dot0[1]) + (Dot0[2] + Dot0[3]);

            T OneOverDeterminant = static_cast<T>(1) / Dot1;
            return Inverse * OneOverDeterminant;
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
            // mat *=  Matrix<T,4, 4>({
            //             {1, 0, 0, 0},
            //             {0, 1, 0, 0},
            //             {0, 0, 1, 0},
            //             {vec.x, vec.y, vec.z, 1}
            //         });

            mat[3][0] = vec.x;
            mat[3][1] = vec.y;
            mat[3][2] = vec.z;
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
                {0, 0, (-(farPlane + nearPlane) / (farPlane - nearPlane)), -1.0f},
                {0, 0, (-(2 * farPlane * nearPlane) / (farPlane - nearPlane)), 0}
            });
        }

        template<typename T>
        constexpr Matrix<T, 4, 4> ortho(const T& left, const T& right, const T& bot, const T& top, const T& nearPlane, const T& farPlane)
        {
            // Каждая фигурная скобка здесь — это СТОЛБЕЦ (Column)
            return Matrix<T, 4, 4>({
                { 2.0f / (right - left), 0.0f, 0.0f, 0.0f },                                 // Столбец 0
                { 0.0f, 2.0f / (top - bot), 0.0f, 0.0f },                                    // Столбец 1
                { 0.0f, 0.0f, -2.0f / (farPlane - nearPlane), 0.0f },                        // Столбец 2
                {
                    -(right + left) / (right - left),                                        // Столбец 3, строка 0
                    -(top + bot) / (top - bot),                                              // Столбец 3, строка 1
                    -(farPlane + nearPlane) / (farPlane - nearPlane),                        // Столбец 3, строка 2
                    1.0f                                                                     // Столбец 3, строка 3
                }
                });
        }

        template<typename T>
        constexpr Matrix<T, 4, 4> lookAt(Vector3<T> eye, Vector3<T> center, Vector3<T> up)
        {
            Vector3<T> z = (eye - center);
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

