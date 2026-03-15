#ifndef NBCOMMON_NBRECT_HPP
#define NBCOMMON_NBRECT_HPP

template <typename T>
struct NbRect
{
    constexpr NbRect() = default;

    NbRect(
        const T x,
        const T y,
        const T width,
        const T height
    )
        : x(x),
          y(y),
          width(width),
          height(height)
    {
    }

    T x = {};
    T y = {};
    T width = {};
    T height = {};

    NB_NODISCARD constexpr NbPoint<T> getTopLeft() const noexcept
    {
        return {x, y};
    }

    NB_NODISCARD constexpr NbPoint<T> getTopRight() const noexcept
    {
        return {x + width, y};
    }

    NB_NODISCARD constexpr NbPoint<T> getBottomLeft() const noexcept
    {
        return {x, y + height};
    }

    NB_NODISCARD constexpr NbPoint<T> getBottomRight() const noexcept
    {
        return {x + width, y + height};
    }

    constexpr NbRect<T> expand(const T& coef) const noexcept
    {
        return NbRect<T>{
            this->x - coef,
            this->y - coef,
            this->width + coef * T(2),
            this->height + coef * T(2),
        };
    }

    constexpr bool isEmpty() const
    {
        return std::fabs(width) < std::numeric_limits<float>::epsilon() ||
               std::fabs(height) < std::numeric_limits<float>::epsilon();
    }

    constexpr void scale(const float scaleFactor)
    {
        x *= scaleFactor;
        y *= scaleFactor;
        width *= scaleFactor;
        height *= scaleFactor;
    }

    constexpr void scaleX(const float scaleFactor) noexcept
    {
        x *= scaleFactor;
        width *= scaleFactor;
    }

    constexpr void scaleY(const float scaleFactor) noexcept
    {
        y *= scaleFactor;
        height *= scaleFactor;
    }

    constexpr void scale(const NbSize<float>& scaleFactor)
    {
        x *= scaleFactor.width;
        x = trunc(x);
        y *= scaleFactor.height;
        y = ceil(y);
        width *= scaleFactor.width;
        height *= scaleFactor.height;
    }

    constexpr NbRect<float> toFloat() const
    {
        return NbRect<float>(
            static_cast<float>(this->x), static_cast<float>(this->y),
            static_cast<float>(this->width), static_cast<float>(this->height)
        );
    }
    template <typename U>
    constexpr NbRect<U> to() const noexcept
    {
        return NbRect<U>(
            static_cast<U>(this->x), static_cast<U>(this->y), static_cast<U>(this->width),
            static_cast<U>(this->height)
        );
    }

    constexpr bool isInside(const NbPoint<T>& point) const noexcept
    {
        return point.x >= this->x && point.x < this->x + this->width && point.y >= this->y &&
               point.y < this->y + this->height;
    }

    constexpr bool operator==(const NbRect<T>& other) const noexcept
    {
        return x == other.x && y == other.y && width == other.width && height == other.height;
    }

    constexpr bool operator!=(const NbRect<T>& other) const noexcept
    {
        return !(*this == other);
    }
};


#endif