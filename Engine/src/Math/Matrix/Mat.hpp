#ifndef SRC_MATH_MATRIX_MAT_HPP
#define SRC_MATH_MATRIX_MAT_HPP

#include <type_traits>
#include <algorithm>


namespace nb
{
    namespace Math
    {
        template <typename T, std::size_t Rows, std::size_t Cols, typename Enable = void>
        class Mat;

        template<typename T, std::size_t Rows, std::size_t Cols>
        class Mat<T, Cols, Rows, typename std::enable_if<std::is_arithmetic<T>::value>::type>
        {
        public:
            constexpr Mat() noexcept
            {
                std::fill_n(*mat, Cols * Rows, 0);
            }

            constexpr Mat(const T value) noexcept
            {
                std::fill_n(*mat, Cols * Rows, value);
            }

            Mat<T, Rows, Cols> operator+(const Mat<T, Rows, Cols>& other) const
            {
                Mat<T, Rows, Cols> res;

                for (size_t i = 0; i < Rows; i++)
                {
                    for (size_t j = 0; j < Cols; j++)
                    {
                        res.mat[i][j] = this->mat[i][j] + other.mat[i][j];
                    }
                }
                return res;
            }

        private:
            T mat[Rows][Cols];
        };
    };
};

#endif