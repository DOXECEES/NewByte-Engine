#ifndef NBSTL_SRC_ITERATOR_HPP
#define NBSTL_SRC_ITERATOR_HPP

#include <cstddef>  // для std::ptrdiff_t
#include <iterator> // для std::random_access_iterator_tag


namespace nbstl 
{
	template<typename Iter>
	struct IteratorTraits
	{
		using DifferenceType = typename Iter::DifferenceType;
		using ValueType = typename Iter::ValueType;
		using Pointer = typename Iter::Pointer;
		using Reference = typename Iter::Reference;
		using IteratorCategory = typename Iter::IteratorCategory;
	};

	template<typename T>
	struct IteratorTraits<T*>
	{
		using DifferenceType = std::ptrdiff_t;
		using ValueType = T;
		using Pointer = T*;
		using Reference = T&;
		using IteratorCategory = Reference; // 
	};

	template<typename T>
	struct IteratorTraits<const T*>
	{
		using DifferenceType = std::ptrdiff_t;
		using ValueType = T;
		using Pointer = const T*;
		using Reference = const T&;
		using IteratorCategory = Reference;
	};


	template<typename Iter>
	class ReverseIterator
	{
	public:
		using IteratorType = Iter;
		using IteratorCategory = std::random_access_iterator_tag;

		using ValueType = typename IteratorTraits<Iter>::ValueType;
		using DifferenceType = typename IteratorTraits<Iter>::DifferenceType;
		using Pointer = typename IteratorTraits<Iter>::Pointer;
		using Reference = typename IteratorTraits<Iter>::Reference;

		constexpr ReverseIterator() noexcept : current() {}
		constexpr explicit ReverseIterator(Iter it) noexcept : current(it) {}

		constexpr Iter base() const noexcept { return current; }

		constexpr Reference operator*() const noexcept
		{
			Iter tmp = current;
			--tmp;
			return *tmp;
		}

		constexpr Pointer operator->() const noexcept
		{
			return addressOf(operator*());
		}

		// ++r  двигает назад
		constexpr ReverseIterator& operator++() noexcept
		{
			--current;
			return *this;
		}

		// r++  двигает назад
		constexpr ReverseIterator operator++(int) noexcept
		{
			ReverseIterator tmp = *this;
			--current;
			return tmp;
		}

		// --r  двигает вперед
		constexpr ReverseIterator& operator--() noexcept
		{
			++current;
			return *this;
		}

		constexpr ReverseIterator operator--(int) noexcept
		{
			ReverseIterator tmp = *this;
			++current;
			return tmp;
		}

		// random access
		constexpr ReverseIterator operator+(DifferenceType n) const noexcept
		{
			return ReverseIterator(current - n);
		}

		constexpr ReverseIterator& operator+=(DifferenceType n) noexcept
		{
			current -= n;
			return *this;
		}

		constexpr ReverseIterator operator-(DifferenceType n) const noexcept
		{
			return ReverseIterator(current + n);
		}

		constexpr ReverseIterator& operator-=(DifferenceType n) noexcept
		{
			current += n;
			return *this;
		}

		constexpr Reference operator[](DifferenceType n) const noexcept
		{
			return *(*this + n);
		}

		// comparisons
		constexpr friend bool operator==(const ReverseIterator& a, const ReverseIterator& b) noexcept
		{
			return a.current == b.current;
		}

		constexpr friend bool operator!=(const ReverseIterator& a, const ReverseIterator& b) noexcept
		{
			return a.current != b.current;
		}

		constexpr friend bool operator<(const ReverseIterator& a, const ReverseIterator& b) noexcept
		{
			return a.current > b.current;
		}

		constexpr friend bool operator>(const ReverseIterator& a, const ReverseIterator& b) noexcept
		{
			return a.current < b.current;
		}

		constexpr friend bool operator<=(const ReverseIterator& a, const ReverseIterator& b) noexcept
		{
			return a.current >= b.current;
		}

		constexpr friend bool operator>=(const ReverseIterator& a, const ReverseIterator& b) noexcept
		{
			return a.current <= b.current;
		}

		constexpr friend DifferenceType operator-(const ReverseIterator& a, const ReverseIterator& b) noexcept
		{
			return b.current - a.current;
		}

		constexpr friend ReverseIterator operator+(DifferenceType n, const ReverseIterator& it) noexcept
		{
			return it + n;
		}

	private:
		Iter current;
	};

}; 

#endif 