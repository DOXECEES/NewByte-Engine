#ifndef SRC_MATH_MATRIX_MATRIX_HPP
#define SRC_MATH_MATRIX_MATRIX_HPP

#include <array>

#include "../Vector2.hpp"
#include "../Vector3.hpp"
#include "../Vector4.hpp"
#include "../Constants.hpp"

#include "../../Core.hpp"

namespace nb
{
    namespace Math
    {
        template <typename T, size_t Rows, size_t Cols>
        class Matrix;

        template <typename T>
        using Mat2 = Matrix<T, 2, 2>;

        template <typename T>
        using Mat3 = Matrix<T, 3, 3>;

        template <typename T>
        using Mat4 = Matrix<T, 4, 4>;

        template <typename T, size_t Rows, size_t Cols>
        class Matrix
        {
            static_assert((Rows >= 2 && Rows <= 4) 
                        && (Cols >= 2 && Cols <= 4), "Matrix size must be between 2x2 and 4x4.");

        public:
            using VecType = std::conditional_t<Cols == 2, Vector2<T>, std::conditional_t<Cols == 3, Vector3<T>, Vector4<T>>>;

            std::array<VecType, Rows> data;

            Matrix()
            {
                for (auto &row : data)
                {
                    row = VecType();
                }
            }

            // Конструктор с параметрами
            Matrix(const std::initializer_list<VecType> &init)
            {
                assert(init.size() == Rows && "Initializer list size must match number of rows.");
                std::copy(init.begin(), init.end(), data.begin());
            }

            VecType &operator[](size_t row)
            {
                assert(row < Rows && "Row index out of bounds.");
                return data[row];
            }

            const VecType &operator[](size_t row) const
            {
                assert(row < Rows && "Row index out of bounds.");
                return data[row];
            }

            const T* valuePtr() const
            {
                return &data[0][0];
            }

            // Операторы сложения
            friend constexpr Matrix<T, Rows, Cols> operator+(Matrix<T, Rows, Cols> a, const Matrix<T, Rows, Cols> &oth) noexcept
            {
                for (size_t i = 0; i < Rows; ++i)
                {
                    a[i] += oth[i];
                }
                return a;
            }

            constexpr Matrix<T, Rows, Cols> &operator+=(const Matrix<T, Rows, Cols> &oth) noexcept
            {
                for (size_t i = 0; i < Rows; ++i)
                {
                    data[i] += oth[i];
                }
                return *this;
            }

            // Операторы вычитания
            friend constexpr Matrix<T, Rows, Cols> operator-(Matrix<T, Rows, Cols> a, const Matrix<T, Rows, Cols> &oth) noexcept
            {
                for (size_t i = 0; i < Rows; ++i)
                {
                    a[i] -= oth[i];
                }
                return a;
            }

            constexpr Matrix<T, Rows, Cols> &operator-=(const Matrix<T, Rows, Cols> &oth) noexcept
            {
                for (size_t i = 0; i < Rows; ++i)
                {
                    data[i] -= oth[i];
                }
                return *this;
            }

            // Операторы умножения на скаляр
            friend constexpr Matrix<T, Rows, Cols> operator*(Matrix<T, Rows, Cols> a, const T scalar) noexcept
            {
                for (size_t i = 0; i < Rows; ++i)
                {
                    a[i] *= scalar;
                }
                return a;
            }

            constexpr Matrix<T, Rows, Cols> &operator*=(const T scalar) noexcept
            {
                for (size_t i = 0; i < Rows; ++i)
                {
                    data[i] *= scalar;
                }
                return *this;
            }

            // Операторы деления на скаляр
            friend constexpr Matrix<T, Rows, Cols> operator/(Matrix<T, Rows, Cols> a, const T scalar) noexcept
            {
                assert(scalar != T(0) && "Division by zero");
                for (size_t i = 0; i < Rows; ++i)
                {
                    a[i] /= scalar;
                }
                return a;
            }

            constexpr Matrix<T, Rows, Cols> &operator/=(const T scalar) noexcept
            {
                assert(scalar != T(0) && "Division by zero");
                for (size_t i = 0; i < Rows; ++i)
                {
                    data[i] /= scalar;
                }
                return *this;
            }

            // Оператор умножения матриц
            template <size_t OtherCols>
            friend constexpr Matrix<T, Rows, OtherCols> operator*(const Matrix<T, Rows, Cols> &lhs, const Matrix<T, Cols, OtherCols> &rhs) noexcept
            {
                // static_assert(Cols == Rows, "Matrix multiplication requires the number of columns in the first matrix to match the number of rows in the second.");

                Matrix<T, Rows, OtherCols> result;
                for (size_t i = 0; i < Rows; ++i)
                {
                    for (size_t j = 0; j < OtherCols; ++j)
                    {
                        T sum = 0;
                        for (size_t k = 0; k < Cols; ++k)
                        {
                            sum += lhs[i][k] * rhs[k][j];
                        }
                        result[i][j] = sum;
                    }
                }
                return result;
            }

            // Оператор умножения на вектор
            template <typename Vec>
            friend constexpr Vec operator*(const Matrix<T, Rows, Cols> &mat, const Vec &vec) noexcept
            {
                static_assert((std::is_same<Vec, Vector2<T>>::value ||
                               std::is_same<Vec, Vector3<T>>::value ||
                               std::is_same<Vec, Vector4<T>>::value),
                              "Vector type must be Vector2, Vector3, or Vector4.");

                Vec result;
                assert(Cols == vec.size() && "Matrix column count must match vector size.");

                for (size_t i = 0; i < Rows; ++i)
                {
                    result[i] = mat[i].dot(vec);
                }
                return result;
            }
        };
    };
}

#endif