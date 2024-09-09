

// TODO

// #ifndef SRC_MATH_VECTOR4_HPP
// #define SRC_MATH_VECTOR4_HPP

// #include <cmath>

// namespace nb
// {
//     namespace Math
//     {
//         class Vector4
//         {

//         public:
//             constexpr Vector4() noexcept = default;

//             constexpr Vector4(const float x, const float y, const float z, const bool w) noexcept
//                 : x(x)
//                 , y(y)
//                 , z(z)
//                 , w(w)
//             {
//             }

//             constexpr Vector4(const Vector4 &oth) noexcept = default;

//             constexpr Vector4 operator+(const Vector4 &oth) noexcept
//             {
//                 return {this->x + oth.x, this->y + oth.y};
//             }
//             constexpr Vector2 operator-(const Vector2 &oth) noexcept
//             {
//                 return {this->x - oth.x, this->y - oth.y};
//             }
//             constexpr Vector2 operator*(const float scalar) noexcept
//             {
//                 return {this->x * scalar, this->y * scalar};
//             }

//             // if division by zero occure return same vector
//             constexpr Vector2 operator/(const float scalar) noexcept
//             {
//                 return static_cast<bool>(scalar) ? Vector2(this->x / scalar, this->y / scalar) : *this;
//             }

//             constexpr float dot(const Vector2 &oth) noexcept
//             {
//                 return {this->x * oth.x + this->y * oth.y};
//             }
//             constexpr float cross(const Vector4 &oth) noexcept
//             {
//                 return {this->x * oth.y - this->y * oth.x};
//             }

//             void normalize() noexcept;
//             float length() noexcept;

//             float x = 0.0f;
//             float y = 0.0f;
//             float z = 0.0f;
//             bool w = false;
//         };

//     };
// };

// #endif