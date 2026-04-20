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
        Matrix<T, 4, 4> inverse(const Matrix<T, 4, 4>& m) 
        {


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

            auto finalInv = Inverse * OneOverDeterminant;

            Matrix<T, 4, 4> result;
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    result[i][j] = finalInv[j][i];
                }
            }
            return result;
        }

        template<typename T>
        Matrix<T, 4, 4> inverseWithoutTranspose(const Matrix<T, 4, 4>& m)
        {

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

            auto finalInv = Inverse * OneOverDeterminant;

            return finalInv;
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
        constexpr Matrix<T, 4, 4> translate(
            Matrix<T, 4, 4> mat,
            const Vector3<T>& vec
        )
        {
            mat *= Matrix<T,4,4>({
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
                {0, 0, (-(farPlane + nearPlane) / (farPlane - nearPlane)), -1.0f},
                {0, 0, (-(2 * farPlane * nearPlane) / (farPlane - nearPlane)), 0}
            });
        }

        template<typename T>
        constexpr Matrix<T, 4, 4> ortho(const T& left, const T& right, const T& bot, const T& top, const T& nearPlane, const T& farPlane)
        {
            return Matrix<T, 4, 4>({
                { 2.0f / (right - left), 0.0f, 0.0f, 0.0f },                                 
                { 0.0f, 2.0f / (top - bot), 0.0f, 0.0f },                                    
                { 0.0f, 0.0f, -2.0f / (farPlane - nearPlane), 0.0f },                        
                {
                    -(right + left) / (right - left),                                        
                    -(top + bot) / (top - bot),                                              
                    -(farPlane + nearPlane) / (farPlane - nearPlane),                        
                    1.0f                                                                     
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

        template <typename T>
        constexpr Quaternion<T> eulerToQuaternion(const Vector3<T>& euler)
        {

            const T cx = std::cos(euler.x * 0.5f);
            const T sx = std::sin(euler.x * 0.5f);
            const T cy = std::cos(euler.y * 0.5f);
            const T sy = std::sin(euler.y * 0.5f);
            const T cz = std::cos(euler.z * 0.5f);
            const T sz = std::sin(euler.z * 0.5f);

            return {
                sx * cy * cz - cx * sy * sz, // x
                cx * sy * cz + sx * cy * sz, // y
                cx * cy * sz - sx * sy * cz, // z
                cx * cy * cz + sx * sy * sz  // w
            };
        }

        inline nb::Math::Vector3<float> quatToEuler(const nb::Math::Quaternion<float>& q)
        {
            nb::Math::Vector3<float> angles;

            double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
            double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
            angles.x = std::atan2(sinr_cosp, cosr_cosp);

            double sinp = 2 * (q.w * q.y - q.z * q.x);
            if (std::abs(sinp) >= 1)
            {
                angles.y =
                    std::copysign(Constants::PI / 2, sinp); // use 90 degrees if out of range
            }
            else
            {
                angles.y = std::asin(sinp);
            }

            double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
            double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
            angles.z = std::atan2(siny_cosp, cosy_cosp);

            return angles; 
        }

        inline Vector3<float> transformPoint(
            const Mat4<float>& m,
            const Vector3<float>& v
        )
        {
            float x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3];
            float y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3];
            float z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3];
            float w = m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3];
            return Vector3<float>(x / w, y / w, z / w);
        }

        inline Vector3<float> transformVector(
            const Mat4<float>& m,
            const Vector3<float>& v
        )
        {
            float x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z;
            float y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z;
            float z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z;
            return Vector3<float>(x, y, z);
        }

        template<typename T>
        constexpr Matrix<T, 4, 4> transform(
            const Vector3<T>& position,
            const nb::Math::Quaternion<T>& rotation,
            const Vector3<T>& scale
        ) noexcept
        {
            Mat4<T> matrix = Mat4<T>::identity();

            matrix         = Math::scale(matrix, scale);
            matrix = matrix * rotation.toMatrix4();
            matrix = Math::translate(matrix, position);
            
            return matrix;
        }

        template<typename T>
        Vector3<T> getPositionFromModelMatrix(const Mat4<T>& model) noexcept
        {
            return {
                model[3][0],
                model[3][1],
                model[3][2]
            };
        }

        template <typename T>
        Quaternion<T> getRotationFromModelMatrix(const Mat4<T>& model) noexcept
        {
            Vector3<T> scale = getScaleFromModelMatrix(model);

            T m00 = model[0][0] / scale.x;
            T m01 = model[0][1] / scale.x;
            T m02 = model[0][2] / scale.x;

            T m10 = model[1][0] / scale.y;
            T m11 = model[1][1] / scale.y;
            T m12 = model[1][2] / scale.y;

            T m20 = model[2][0] / scale.z;
            T m21 = model[2][1] / scale.z;
            T m22 = model[2][2] / scale.z;

            Quaternion<T> q;
            T             trace = m00 + m11 + m22;

            if (trace > 0.0f)
            {
                T s = 0.5f / std::sqrt(trace + 1.0f);
                q.w = 0.25f / s;
                q.x = (m21 - m12) * s;
                q.y = (m02 - m20) * s;
                q.z = (m10 - m01) * s;
            }
            else
            {
                if (m00 > m11 && m00 > m22)
                {
                    T s = 2.0f * std::sqrt(1.0f + m00 - m11 - m22);
                    q.w = (m21 - m12) / s;
                    q.x = 0.25f * s;
                    q.y = (m01 + m10) / s;
                    q.z = (m02 + m20) / s;
                }
                else if (m11 > m22)
                {
                    T s = 2.0f * std::sqrt(1.0f + m11 - m00 - m22);
                    q.w = (m02 - m20) / s; 
                    q.x = (m01 + m10) / s;
                    q.y = 0.25f * s;
                    q.z = (m12 + m21) / s;
                }
                else
                {
                    T s = 2.0f * std::sqrt(1.0f + m22 - m00 - m11);
                    q.w = (m10 - m01) / s; 
                    q.x = (m02 + m20) / s;
                    q.y = (m12 + m21) / s;
                    q.z = 0.25f * s;
                }
            }
            q.normalize();
            return q;
        }


        template <typename T>
        Vector3<T> getScaleFromModelMatrix(const Mat4<T>& model) noexcept
        {
            return {
                std::sqrt(
                    model[0][0] * model[0][0] + model[0][1] * model[0][1] +
                    model[0][2] * model[0][2]
                ),
                std::sqrt(
                    model[1][0] * model[1][0] + model[1][1] * model[1][1] +
                    model[1][2] * model[1][2]
                ),
                std::sqrt(
                    model[2][0] * model[2][0] + model[2][1] * model[2][1] +
                    model[2][2] * model[2][2]
                )
            };
        }

        template <typename T>
        constexpr Quaternion<T> inverse(const Quaternion<T>& q) noexcept
        {
            T normSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;

            if (normSq > 0)
            {
                T invNormSq = static_cast<T>(1.0) / normSq;
                return Quaternion<T>{
                    -q.x * invNormSq, -q.y * invNormSq, -q.z * invNormSq, q.w * invNormSq
                };
            }
            return Quaternion<T>{0, 0, 0, 1}; 
        }
    };
};

#endif

