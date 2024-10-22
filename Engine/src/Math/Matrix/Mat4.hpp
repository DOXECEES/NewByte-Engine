#ifndef SRC_MATH_MATRIX_MAT4_HPP
#define SRC_MATH_MATRIX_MAT4_HPP

#include <type_traits>
#include <algorithm>

namespace nb
{
    namespace Math
    {
        template <typename T, typename Enable = void>
        class Mat4;

        template<typename T>
        class Mat4<T,std::enable_if<std::is_arithmetic<T>::value>::type>
        {

        public:
            constexpr Mat4() noexcept = default;
            constexpr Mat4(const Mat4& other) noexcept = default;
            constexpr Mat4(Mat4&& other) noexcept = default;
            ~Mat4() = default;
            Mat4& operator=(const Mat4& other) noexcept = default;
            Mat4& operator=(Mat4&& other) noexcept = default;
           
            constexpr Mat4(const T value) noexcept
                :mat(std::fill_n(*mat, 16, value))
            {

            }

        private:
            T mat[4][4] = {
                {1,0,0,0},
                {0,1,0,0},
                {0,0,1,0},
                {0,0,0,1}
            };
        };
    };
};

#endif