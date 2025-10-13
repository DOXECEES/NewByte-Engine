#ifndef NBSTL_SRC_BITMANIPULATION_HPP
#define NBSTL_SRC_BITMANIPULATION_HPP


#include <cstdint>
#include <type_traits>
#include <cassert>

namespace nbstl
{
	template<typename T>
	bool getHighBit(const T value) noexcept
	{
		static_assert(std::is_integral_v<T>, "T must be an integral type");
		return (value & (T(1) << ((sizeof(T) * 8) - 1))) != 0;
	}

	template<typename T>
	bool getLowBit(const T value) noexcept
	{
		static_assert(std::is_integral_v<T>, "T must be an integral type");
		return (value & 1) != 0;
	}

	template<typename T>
	bool getBit(const T value, size_t index) noexcept
	{
		static_assert(std::is_integral_v<T>, "T must be an integral type");
		assert(index < sizeof(T) * 8);
		return (value & (T(1) << index)) != 0;
	}

	template<typename T>
	void setBit(T& value, size_t index) noexcept
	{
		static_assert(std::is_integral_v<T>, "T must be an integral type");
		assert(index < sizeof(T) * 8);
		value |= (T(1) << index);
	}

	template<typename T>
	void clearBit(T& value, size_t index) noexcept
	{
		static_assert(std::is_integral_v<T>, "T must be an integral type");
		assert(index < sizeof(T) * 8);
		value &= ~(T(1) << index);
	}

	template<typename T>
	void toggleBit(T& value, size_t index) noexcept
	{
		static_assert(std::is_integral_v<T>, "T must be an integral type");
		assert(index < sizeof(T) * 8);
		value ^= (T(1) << index);
	}
}

#endif
