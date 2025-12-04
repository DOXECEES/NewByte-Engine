#ifndef NBSTL_SRC_MEMORY_HPP
#define NBSTL_SRC_MEMORY_HPP

#include "TypeTraits.hpp"

namespace nbstl 
{
	template <typename T>
	constexpr T&& forward(typename removeReference<T>::type& arg) noexcept
	{
		return static_cast<T&&>(arg);
	}

	template<typename T>
	constexpr T* addressOf(T& ref) noexcept
	{
		return reinterpret_cast<T*>(
			&const_cast<char&>(
				reinterpret_cast<const volatile char&>(ref)
				)
			);
	}
	
	template<typename T, typename... Args>
	constexpr T* constructAt(T* p, Args&&... args) noexcept
	{
		return ::new (static_cast<void*>(p)) T(forward<Args>(args)...);
	}

	template<typename T>
	constexpr void destroyAt(T* p) noexcept
	{
		if constexpr (isArray<T>::value)
		{
			for (auto& elem : *p)
			{
				destroyAt(addressOf(elem));
			}
		}
		else
		{
			p->~T();
		}
	}

	template<typename T>
	constexpr T&& move(T& obj) noexcept
	{
		return static_cast<T&&>(obj);
	}
}; 

#endif 
