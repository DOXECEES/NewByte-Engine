#ifndef NBSTL_SRC_NBMATH_MATH_HPP
#define NBSTL_SRC_NBMATH_MATH_HPP

#include <NbCore.hpp>
#include <type_traits>

namespace nbstl
{
    constexpr static double DEFAULT_EPSILON = 1e-6;

	namespace Math
	{
        template<std::floating_point T>
        NB_NODISCARD
        constexpr bool equalByEpsilon(
            T first,
            T second,
            T epsilon = static_cast<T>(DEFAULT_EPSILON)
        ) noexcept
        {
            const T diff = first - second;
            const T absoluteDiff = (diff < 0) ? -diff : diff;

            return absoluteDiff < epsilon;
        }
	};
};

#endif