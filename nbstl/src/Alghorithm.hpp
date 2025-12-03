#ifndef NBSTL_SRC_ALGHORITHM_HPP
#define NBSTL_SRC_ALGHORITHM_HPP

#undef max
#undef min

#include "TypeTraits.hpp"

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

    template<typename T>
    constexpr T clamp(const T& value, const T& minValue, const T& maxValue) noexcept
    {
        return min(maxValue, max(minValue, value));
    }

    template<typename T>
    constexpr T clampMax(const T& value, const T& maxValue) noexcept
    {
        return min(value, maxValue);
    }

    template<typename T>
    constexpr T clampMin(const T& value, const T& minValue) noexcept
    {
        return max(value, minValue);
    }

    template<typename T>
    constexpr T saturate(const T& value) noexcept
    {
        return clamp(value, T(0), T(1));
    }


 



    // --- базовые утилиты типов ---



    // --- проверка на наличие begin() ---
    template<typename T, typename = void>
    struct hasBegin : falseType {};

    template<typename T>
    struct hasBegin<T, voidT<decltype(declval<T&>().begin())>> : trueType {};

    // --- проверка на наличие end() ---
    template<typename T, typename = void>
    struct hasEnd : falseType {};

    template<typename T>
    struct hasEnd<T, voidT<decltype(declval<T&>().end())>> : trueType {};

    // --- проверка, является ли что-то контейнером ---


    // --- базовая функция для получения диапазона ---
    /*template<typename Container>
    auto toIteratorPair(Container& c)
    {
        return nbstl::pair{ c.begin(), c.end() };
    }*/

    template<typename T1, typename T2>
    struct Pair
    {
        T1 first;
        T2 second;

        constexpr Pair() = default;
        constexpr Pair(T1 f, T2 s) : first(f), second(s) {}
    };

    // --- упрощённая пара ---
    template<typename A, typename B>
    struct pair
    {
        A first;
        B second;

        pair() : first(), second() {}
        pair(const A& a, const B& b) : first(a), second(b) {}
    };

    // --- ForEach для контейнера ---
    template<typename Container, typename Func>
    inline typename enableIf<isIterable<Container>::value>::type
        forEach(Container& c, Func f)
    {
        auto it = c.begin();
        auto end = c.end();
        while (it != end)
        {
            f(*it);
            ++it;
        }
    }

    // --- ForEach для пары итераторов ---
    template<typename Iterator, typename Func>
    inline void forEach(Iterator first, Iterator last, Func f)
    {
        while (first != last)
        {
            f(*first);
            ++first;
        }
    }
    

    //

       // math

    template<typename T, typename U>
    constexpr T lerp(const T& a, const T& b, const U& t) noexcept
    {
        return a * (U(1) - t) + b * t;
    }

    template<typename T>
    constexpr T smoothstep(T value, const T& minValue, const T& maxValue) noexcept
    {
        value = saturate((value - minValue) / (maxValue - minValue));
        return value * value * (T(3) - T(2) * value);
    }


    template<typename T, typename = enableIfT<isIntegral<T>>>
    constexpr T myAbs(const T& value) noexcept
    {
        return value >= T(0) ? value : -value;
    }

    template<typename T, typename = enableIfT<isIntegral<T>>>
    constexpr int sign(const T& value) noexcept
    {
        return (value > 0) - (value < 0);
    }

    template<typename T, typename = enableIfT<isFloatingPoint<T>>>
    constexpr T floor(const T& value) noexcept
    {
        T intPart = T(static_cast<long long>(value));
        return (value < T(0) && value != intPart) ? intPart - T(1) : intPart;
    }

    template<typename T, typename = enableIfT<isFloatingPoint<T>>>
    constexpr T ceil(const T& value) noexcept
    {
        T intPart = T(static_cast<long long>(value)); 
        return (value > T(0) && value != intPart) ? intPart + T(1) : intPart;
    }

    template<typename T, typename = enableIfT<isFloatingPoint<T>>>
    constexpr T round(T value) noexcept
    {
        return floor(value + T(0.5));
    }

    template<typename T, typename = enableIfT<isFloatingPoint<T>>>
    constexpr auto mod(const T& value) noexcept
    {
        T intPart = floor(value);          
        T fracPart = value - intPart;
        return Pair<T, T>{intPart, fracPart}; 
    }

    template<typename T, typename = enableIfT<isFloatingPoint<T>>>
    constexpr T truncate(const T& value) noexcept
    {
        return floor(value);
    }

    template<typename T, typename = enableIfT<isFloatingPoint<T>>>
    constexpr auto fraction(const T& value) noexcept
    {
        ;
        return value - truncate(value);
    }

    template<typename T, typename = enableIfT<isFloatingPoint<T>>>
    constexpr T sin(const T& x) noexcept
    {
        using StdT = double;
        return static_cast<T>(std::sin(static_cast<StdT>(x)));
    }

    template<typename T, typename = enableIfT<isFloatingPoint<T>>>
    constexpr T cos(const T& x) noexcept 
    {
        using StdT = double;
        return static_cast<T>(std::cos(static_cast<StdT>(x)));
    }

    template<typename T, typename = enableIfT<isFloatingPoint<T>>>
    constexpr T tan(const T& x) noexcept
    {
        using StdT = double;
        return static_cast<T>(std::tan(static_cast<StdT>(x)));
    }

    template<typename T, typename = enableIfT<isFloatingPoint<T>>>
    constexpr T asin(const T& x) noexcept 
    {
        using StdT = double;
        return static_cast<T>(std::asin(static_cast<StdT>(x)));
    }

    template<typename T, typename = enableIfT<isFloatingPoint<T>>>
    constexpr T acos(const T& x) noexcept
    {
        using StdT = double;
        return static_cast<T>(std::acos(static_cast<StdT>(x)));
    }

    template<typename T, typename = enableIfT<isFloatingPoint<T>>>
    constexpr T atan(const T& x) noexcept
    {
        using StdT = double;
        return static_cast<T>(std::atan(static_cast<StdT>(x)));
    }

    template<typename T, typename = enableIfT<isFloatingPoint<T>>>
    constexpr T atan2(const T& y, const T& x) noexcept
    {
        using StdT = double;
        return static_cast<T>(std::atan2(static_cast<StdT>(y),
            static_cast<StdT>(x)));
    }

    template<typename T, typename = enableIfT<isFloatingPoint<T>>>
    constexpr T sinh(const T& x) noexcept 
    {
        using StdT = double;
        return static_cast<T>(std::sinh(static_cast<StdT>(x)));
    }

    template<typename T, typename = enableIfT<isFloatingPoint<T>>>
    constexpr T cosh(const T& x) noexcept 
    {
        using StdT = double;
        return static_cast<T>(std::cosh(static_cast<StdT>(x)));
    }

    template<typename T, typename = enableIfT<isFloatingPoint<T>>>
    constexpr T tanh(const T& x) noexcept
    {
        using StdT = double;
        return static_cast<T>(std::tanh(static_cast<StdT>(x)));
    }

    template<typename T, typename U>
    constexpr T pow(const T& base, const U& exp) noexcept
    {
        return std::pow(base, exp);
    }

    template<typename T>
    constexpr T sqrt(const T& value) noexcept
    {
        return std::sqrt(value);
    }




};




#endif