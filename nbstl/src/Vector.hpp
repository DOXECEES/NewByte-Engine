#ifndef NBSTL_SRC_VECTOR_HPP
#define NBSTL_SRC_VECTOR_HPP

#include "NbCore.hpp"
#include "Types.hpp"
#include "Iterator.hpp"
#include "Memory.hpp"

#include <initializer_list> // cannot be rewriten

namespace nbstl 
{
	template<typename T>
	class Vector
	{
		constexpr static Size DEFAULT_SIZE = 8;
	public:

		using Iterator = T*;
		using ConstIterator = const T*;
		using ReverseIterator = nbstl::ReverseIterator<T*>;
		using ConstReverseIterator = nbstl::ReverseIterator<const T*>;
		using Reference = T&;
		using ConstReference = const T&;



		Vector() noexcept
		{
			_begin = static_cast<T*>(::operator new(sizeof(T) * DEFAULT_SIZE));
			_end = _begin;
			_capacityEnd = _begin + DEFAULT_SIZE;
		}

		Vector(const Vector& other) noexcept
		{
			Size n = other.size();
			_begin = static_cast<T*>(::operator new(sizeof(T) * (n > DEFAULT_SIZE ? n : DEFAULT_SIZE)));
			_end = _begin;
			_capacityEnd = _begin + (n > DEFAULT_SIZE ? n : DEFAULT_SIZE);

			for (Size i = 0; i < n; ++i)
				constructAt(_end++, other._begin[i]);
		}

		Vector& operator=(const Vector& other) noexcept
		{
			if (this != &other)
			{
				clear();
				reserve(other.size());
				for (Size i = 0; i < other.size(); ++i)
				{
					pushBack(other._begin[i]);
				}
			}
			return *this;
		}

		Vector(Vector&& other) noexcept
			: _begin(other._begin), _end(other._end), _capacityEnd(other._capacityEnd)
		{
			other._begin = other._end = other._capacityEnd = nullptr;
		}

		Vector& operator=(Vector&& other) noexcept
		{
			if (this != &other)
			{
				clear();
				::operator delete(_begin);

				_begin = other._begin;
				_end = other._end;
				_capacityEnd = other._capacityEnd;

				other._begin = other._end = other._capacityEnd = nullptr;
			}
			return *this;
		}

		Vector(std::initializer_list<T> init)
		{
			reserve(init.size());
			for (const T& value : init)
			{
				pushBack(value);
			}
		}


		~Vector() noexcept
		{
			for (T* p = _begin; p < _end; ++p)
			{
				destroyAt(p);
			}
			::operator delete(_begin);
		}

		void pushBack(const T& value) noexcept
		{
			if (_end == _capacityEnd)
			{
				grow();
			}
			constructAt(_end, value);
			_end++;
		}

		void popBack() noexcept
		{
			_end--;
			destroyAt(_end);
		}



		// Iterators

		Iterator begin() noexcept				{ return _begin; }
		ConstIterator begin() const noexcept	{ return _begin; }
		ConstIterator cbegin() const noexcept	{ return _begin; }
		Iterator end() noexcept					{ return _end; }
		ConstIterator end() const noexcept		{ return _end; }
		ConstIterator cend() const noexcept		{ return _end; }

		ReverseIterator rbegin() noexcept				{ return ReverseIterator(_end); }
		ConstReverseIterator rbegin() const noexcept	{ return ReverseIterator(_end); }
		ConstReverseIterator crbegin() const noexcept	{ return ReverseIterator(_end); };
		ReverseIterator rend() noexcept					{ return ReverseIterator(_begin); }
		ConstReverseIterator rend() const noexcept		{ return ReverseIterator(_begin); }
		ConstReverseIterator crend() const noexcept		{ return ReverseIterator(_begin); };

		// Capacity 
		constexpr bool isEmpty() const noexcept
		{
			return _begin == _end;
		}
		constexpr Size size() const noexcept { return _end - _begin; }
		constexpr Size capacity() const noexcept { return _capacityEnd - _begin; }
		constexpr Size maxSize() const noexcept { return (~Size() - Size()) / sizeof(T); }
		constexpr void reserve(Size newCapacity) noexcept
		{
			if (capacity() >= newCapacity)
			{
				return;
			}

			Size oldSize = size();
			T* newData = static_cast<T*>(::operator new(sizeof(T) * newCapacity));

			for (Size i = 0; i < oldSize; ++i)
			{
				constructAt(newData + i, move(_begin[i]));
				destroyAt(_begin + i);
			}

			if (!isEmpty())
			{
				::operator delete(_begin);
			}
			_begin = newData;
			_end = newData + oldSize;
			_capacityEnd = newData + newCapacity;
		}

		constexpr void shrinkToFit() noexcept
		{
			Size oldSize = size();
			Size newCapacity = capacity() * 2;
			T* newData = static_cast<T*>(::operator new(sizeof(T) * newCapacity));

			for (Size i = 0; i < oldSize; ++i)
			{
				constructAt(newData + i, move(_begin[i]));
				destroyAt(_begin + i);
			}

			::operator delete(_begin);
			_begin = newData;
			_end = newData + oldSize;
			_capacityEnd = newData + newCapacity;
		}
		// Element access

		constexpr Reference at(Size index) noexcept
		{
			NB_ASSERT(index < size(), "INDEX OUT OF RANGE");
			return _begin[index];
		}

		constexpr ConstReference at(Size index) const noexcept
		{
			NB_ASSERT(index < size(), "INDEX OUT OF RANGE");
			return _begin[index];
		}

		constexpr Reference operator[](Size index) noexcept
		{
			return _begin[index];
		}

		constexpr ConstReference operator[](Size index) const noexcept
		{
			return _begin[index];
		}

		constexpr Reference front() noexcept
		{
			NB_ASSERT(!isEmpty(), "Vector is empty, cannot access front()");
			return *_begin;
		}

		constexpr ConstReference front() const noexcept
		{
			NB_ASSERT(!isEmpty(), "Vector is empty, cannot access front()");
			return *_begin;
		}

		constexpr Reference back() noexcept
		{
			NB_ASSERT(!isEmpty(), "Vector is empty, cannot access back()");
			return *(_end - 1);
		}

		constexpr ConstReference back() const noexcept
		{
			NB_ASSERT(!isEmpty(), "Vector is empty, cannot access back()");
			return *(_end - 1);
		}

		constexpr T* data() noexcept
		{
			return !isEmpty() ? _begin : nullptr;
		}

		constexpr const T* data() const noexcept
		{
			return !isEmpty() ? _begin : nullptr;
		}

		// Modifiers
		constexpr void clear() noexcept
		{
			while (_end != _begin)
			{
				_end--;
				destroyAt(_end);
			}
		}

		constexpr Iterator insert(Iterator pos, const T& value) noexcept
		{
			if (_end == _capacityEnd)
			{
				auto index = pos - _begin;
				grow();
				pos = _begin + index;
			}

			T* newEnd = _end + 1;
			for (T* p = _end; p != pos; --p)
			{
				constructAt(p, move(*(p - 1)));
				destroyAt(p - 1);
			}

			constructAt(pos, value);

			_end = newEnd;
			return pos;
		}

		constexpr Iterator insert(Iterator pos, T&& value) noexcept
		{
			if (_end == _capacityEnd)
			{
				auto index = pos - _begin;
				grow();
				pos = _begin + index;
			}

			T* newEnd = _end + 1;
			for (T* p = _end; p != pos; --p)
			{
				constructAt(p, move(*(p - 1)));
				destroyAt(p - 1);
			}

			constructAt(pos, move(value));

			_end = newEnd;
			return pos;
		}

		constexpr Iterator insert(Iterator pos, Size count, const T& value) noexcept
		{
			Size oldSize = size();
			Size oldCapacity = capacity();

			if (oldCapacity - oldSize < count)
			{
				auto index = pos - _begin;

				Size newCapacity = oldCapacity;
				do {
					newCapacity *= 2;
				} while (newCapacity - oldSize < count);

				grow(newCapacity);
				pos = _begin + index;
			}

			for (T* p = _end - 1; p >= pos; --p)
			{
				constructAt(p + count, move(*p));
				destroyAt(p);
			}

			for (Size i = 0; i < count; ++i)
			{
				constructAt(pos + i, value);
			}

			_end += count;

			return pos;
		}

		//insert(Iterator pos, Iterator first, Iterator last)

	private:

		void grow(Size newCap = 0) noexcept
		{
			Size oldSize = size();
			Size newCapacity = capacity() * 2;
			if (newCap != 0)
			{
				newCapacity = newCap;
			}

			T* newData = static_cast<T*>(::operator new(sizeof(T) * newCapacity));

			for (Size i = 0; i < oldSize; ++i)
			{
				constructAt(newData + i, move(_begin[i]));
				destroyAt(_begin + i);
			}

			::operator delete(_begin);
			_begin = newData;
			_end = newData + oldSize;
			_capacityEnd = newData + newCapacity;
		}

		T* _begin;
		T* _end;
		T* _capacityEnd;
	};
}; 

#endif 
