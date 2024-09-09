#include "Vector2.hpp"

void nb::Math::Vector2::normalize() noexcept
{
    const float length = this->length();
    if (length != 0)
    {
        this->x = this->x / length;
        this->y = this->y / length;
    }
}

float nb::Math::Vector2::length() noexcept
{
    return std::sqrtf(dot(*this));
}
