#ifndef NBSTL_SRC_ALGHORITHM_HPP
#define NBSTL_SRC_ALGHORITHM_HPP

#undef max
#undef min


namespace nbstl
{
    template<typename T>
    constexpr T max(T first, T second) noexcept
    {
        return (first < second) ? second : first;
    }

    template<typename T, typename... Args>
    constexpr T maxInRange(T first, Args... args) noexcept
    {
        ((first = (first < args ? args : first)), ...);
        return first;
    }

    template<typename T>
    constexpr T min(T first, T second) noexcept
    {
        return (first > second) ? second : first;
    }

    template<typename T, typename... Args>
    constexpr T minInRange(T first, Args... args) noexcept
    {
        ((first = (first > args ? args : first)), ...);
        return first;
    }

};


#endif