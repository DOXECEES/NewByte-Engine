#ifndef NBSTL_SRC_TYPETRAITS_HPP
#define NBSTL_SRC_TYPETRAITS_HPP

namespace nbstl
{
    template<bool B, typename T = void>
    struct enableIf {};

    template<typename T>
    struct enableIf<true, T> { using type = T; };

    template<typename T>
    using enableIfT = typename enableIf<T::value, void>::type;

    struct trueType { static constexpr bool value = true; };
    struct falseType { static constexpr bool value = false; };

    template<typename T>
    T&& declval();

    template<typename...>
    using voidT = void;

    template<typename T, typename = void>
    struct isIterable : falseType {};

    template<typename T>
    struct isIterable<T, voidT<
        decltype(declval<T&>().begin()),
        decltype(declval<T&>().end())
        >> : trueType {};

    template<typename T> struct isIntegral { static constexpr bool value = false; };

    template<> struct isIntegral<int> { static constexpr bool value = true; };
    template<> struct isIntegral<unsigned int> { static constexpr bool value = true; };
    template<> struct isIntegral<short> { static constexpr bool value = true; };
    template<> struct isIntegral<unsigned short> { static constexpr bool value = true; };
    template<> struct isIntegral<long> { static constexpr bool value = true; };
    template<> struct isIntegral<unsigned long> { static constexpr bool value = true; };
    template<> struct isIntegral<long long> { static constexpr bool value = true; };
    template<> struct isIntegral<unsigned long long> { static constexpr bool value = true; };
    template<> struct isIntegral<char> { static constexpr bool value = true; };
    template<> struct isIntegral<unsigned char> { static constexpr bool value = true; };
    template<> struct isIntegral<signed char> { static constexpr bool value = true; };
    template<> struct isIntegral<wchar_t> { static constexpr bool value = true; };

    template<typename T> struct isFloatingPoint { static constexpr bool value = false; };
    template<> struct isFloatingPoint<float> { static constexpr bool value = true; };

    template<typename T>
    struct isArithmetic
    {
        static constexpr bool value = isIntegral<T>::value || isFloatingPoint<T>::value;
    };

    template<typename T>
    struct removeConst
    {
        using type = T;
    };

    template<typename T>
    struct removeConst<const T>
    {
        using type = T;
    };

    template<typename T>
    struct removeVolatile
    {
        using type = T;
    };

    template<typename T>
    struct removeVolatile<volatile T>
    {
        using type = T;
    };

    template<typename T>
    struct removeConstVolatile
    {
        using type = typename removeConst<typename removeVolatile<T>::type>::type;
    };

    template<typename T>
    using removeConstVolatileT = typename removeConstVolatile<T>::type;

    template<typename T> struct isArray { static constexpr bool value = false; };
    template<typename T> struct isArray<T[]> { static constexpr bool value = true; };
    template<typename T, size_t N> struct isArray<T[N]> { static constexpr bool value = true; };

    template <typename T> struct removeReference { using type = T; };
    template <typename T> struct removeReference<T&> { using type = T; };
    template <typename T> struct removeReference<T&&> { using type = T; };

    template<typename T>
    struct isLvalueReference : falseType {};

    template<typename T>
    struct isLvalueReference<T&> : trueType {};


}


#endif